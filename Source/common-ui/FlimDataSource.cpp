#include "FlimDataSource.h"
#include "LifetimeDisplayWidget.h"
#include "Cv3dUtils.h"
#include "FlimCubeWriter.h"

#include <memory>

FlimDataSource::FlimDataSource(const QString& filename_, QObject* parent) :
   FlimDataSource(parent)
{
   filename = filename_;
   reader = std::shared_ptr<FlimReader>(FlimReader::createReader(filename.toStdString()));

   init();
}

FlimDataSource::FlimDataSource(QObject* parent) :
   RealignableDataSource(parent)
{
}

void FlimDataSource::init()
{
   worker = new DataSourceWorker(this);
   connect(this, &QObject::destroyed, worker, &QObject::deleteLater);

   int n_chan = reader->getNumChannels();
}

FlimDataSource::~FlimDataSource()
{
   terminate = true;
   currently_reading = false;
   reader->stopReading();

   QMetaObject::invokeMethod(worker, "stop");

   std::lock_guard<std::mutex> lk(read_mutex);

   if (reader_thread.valid())
      reader_thread.get();
}

cv::Mat FlimDataSource::getIntensity()
{
   std::lock_guard<std::mutex> lk(image_mutex);
   return intensity;
};

cv::Mat FlimDataSource::getMeanArrivalTime()
{
   std::lock_guard<std::mutex> lk(image_mutex);
   return mean_arrival_time;
};

double FlimDataSource::getTimeResolution()
{
   auto& t = reader->getTimepoints();
   return (t.size() > 0) ? (t[1] - t[0]) : 1;
}


void FlimDataSource::update()
{
   if (terminate)
      return;

   if (task)
   {
      double progress = reader->getProgress();
      task->setProgress(progress);
   }

   cv::Mat intensitybuf, marbuf;
   {
      std::lock_guard<std::mutex> lk(read_mutex);

      if (!currently_reading) return;
      if (!data || !data->isReady()) return;

      data->getIntensityAndMeanArrival(intensitybuf, marbuf);

      // Apply intensity normalisation
      cv::Mat intensity_normalisation = reader->getFloatIntensityNormalisation();
      if (matsSameSizeType(intensity_normalisation, intensitybuf))
         cv::divide(intensitybuf, intensity_normalisation, intensitybuf);
   }

   {
      std::lock_guard<std::mutex> lk_im(image_mutex);
      intensitybuf.copyTo(intensity);
      marbuf.copyTo(mean_arrival_time);
   }

   emit decayUpdated();
}

void FlimDataSource::setupForRead()
{
   std::lock_guard<std::mutex> lk(read_mutex);

   connect(task.get(), &TaskProgress::cancelRequested, this, &FlimDataSource::cancelRead);

   reader->clearStopSignal();

   reader->determineDimensions();
   
   int n_px = reader->dataSizePerChannel();
   int n_chan = reader->getNumChannels();
   int n_x = reader->getNumX();
   int n_y = reader->getNumY();
   int n_z = reader->getNumZ();

   {
      std::vector<int> dims = {n_z, n_y, n_x};
      std::lock_guard<std::mutex> lk(image_mutex);
      intensity = cv::Mat(dims, CV_32F, cv::Scalar(0));
      mean_arrival_time = cv::Mat(dims, CV_32F, cv::Scalar(0));
   }

   data = std::make_shared<FlimCube>();
}

void FlimDataSource::alignFrames()
{
   try
   {
      reader->alignFrames();
   }
   catch (std::runtime_error e)
   {
      emit error(e.what());
   }
}

void FlimDataSource::readAlignedData()
{
   try
   {
      reader->readData(data);
   }
   catch (std::runtime_error e)
   {
      emit error(e.what());
   }
}

void FlimDataSource::cancelRead()
{
   reader->stopReading(); 
   terminate = true;
}


QWidget* FlimDataSource::getWidget()
{
   auto widget = new LifetimeDisplayWidget;
   widget->setFlimDataSource(this);
   return widget;
}

void FlimDataSource::saveData(const QString& root_name, bool interpolate)
{
   auto tags = reader->getTags();
   auto reader_tags = reader->getReaderTags();
   auto images = reader->getImageMap();

   for (int z = 0; z<reader->getNumZ(); z++)
   {
      QString filename;
      if (reader->getNumZ() > 1)
         filename = QString("%1_%2.ffh").arg(root_name).arg(z, 3, 10, QChar('0'));
      else
         filename = QString("%1.ffh").arg(root_name);
      FlimCubeWriter writer(filename.toStdString(), data, z, tags, reader_tags, images);      
   }
}

void FlimDataSource::savePreview(const QString& filename)
{
   writeScaledImage(filename.toStdString(), getIntensity());   
}