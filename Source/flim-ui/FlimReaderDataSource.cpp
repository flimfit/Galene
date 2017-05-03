#include "FlimReaderDataSource.h"
#include <memory>

void FlimReaderDataSourceWorker::update()
{
   if (executing)
   {
      source->update();
      emit updateComplete();
   }
}


FlimReaderDataSource::FlimReaderDataSource(const QString& filename_, QObject* parent) :
   FlimDataSource(parent)
{
   filename = filename_;
   reader = std::shared_ptr<FLIMReader>(FLIMReader::createReader(filename.toStdString()));

   reader->setTemporalResolution(8); // TODO

   worker = new FlimReaderDataSourceWorker(nullptr, this);
   //auto& c = connect(worker, &FlimReaderDataSourceWorker::updateComplete, [&]() { emit decayUpdated(); });
   //conn = std::unique_ptr<QMetaObject::Connection>(&c);

   connect(this, &QObject::destroyed, worker, &QObject::deleteLater);

   int n_chan = reader->getNumChannels();
   count_rates_dummy.resize(n_chan);
}

FlimReaderDataSource::~FlimReaderDataSource()
{
   terminate = true;
   currently_reading = false;
   reader->stopReading();

   //disconnect(*conn);

   worker->stop();

   std::lock_guard<std::mutex> lk(read_mutex);

   if (reader_thread.joinable())
      reader_thread.join();
}

cv::Mat FlimReaderDataSource::getIntensity()
{
   std::lock_guard<std::mutex> lk(image_mutex);
   return intensity;
};

cv::Mat FlimReaderDataSource::getMeanArrivalTime()
{
   std::lock_guard<std::mutex> lk(image_mutex);
   return mean_arrival_time;
};

void FlimReaderDataSource::readData(bool realign)
{
   if (currently_reading)
   {
      //read_again_when_finished = true;
   }
   else
   {
      if (reader_thread.joinable())
         reader_thread.join();

      reader_thread = std::thread(&FlimReaderDataSource::readDataThread, this, realign);
   }
}


void FlimReaderDataSource::update()
{
   if (terminate)
      return;

   {
      std::lock_guard<std::mutex> lk(read_mutex);

      if (!currently_reading) return;
      if (!data->isReady()) return;

      std::lock_guard<std::mutex> lk_im(image_mutex);

      int n_px = reader->numX() * reader->numY();
      int n_chan = reader->getNumChannels();
      int n_t = (int)data->timepoints.size();

      if (intensity.size().area() < n_px) return;

      uint16_t* data_ptr = data->getDataPtr();
      for (int p = 0; p < n_px; p++)
      {
         uint16_t I = 0;
         float It = 0;
         for (int c = 0; c < n_chan; c++)
            for (int t = 0; t < n_t; t++)
            {
               I += *data_ptr;
               It += (*data_ptr) * data->timepoints[t];

               data_ptr++;
            }
         intensity.at<float>(p) = I;
         mean_arrival_time.at<float>(p) = It / I;

      }

      // Apply intensity normalisation
      cv::Mat intensity_normalisation = reader->getIntensityNormalisation();
      if (!intensity_normalisation.empty())
      {
         cv::Mat norm;
         double mn, mx;
         cv::minMaxIdx(intensity_normalisation, &mn, &mx);
         intensity_normalisation.convertTo(norm, CV_32F, 1/mx);
         cv::divide(intensity, norm, intensity);
      }
   }

   QMetaObject::invokeMethod(this, "decayUpdated");
}

// Use readData to call 
void FlimReaderDataSource::readDataThread(bool realign)
{
   int n_px = reader->dataSizePerChannel();
   int n_chan = reader->getNumChannels();
   int n_x = reader->numX();
   int n_y = reader->numY();

   {
      std::lock_guard<std::mutex> lk(image_mutex);
      intensity = cv::Mat(n_y, n_x, CV_32F, cv::Scalar(0));
      mean_arrival_time = cv::Mat(n_y, n_x, CV_32F, cv::Scalar(0));
   }

   data = std::make_shared<FlimCube<uint16_t>>();

   read_again_when_finished = false;
   currently_reading = true;

   try
   {
      if (realign)
      {
         reader->alignFrames();
         emit alignmentComplete();
      }

      reader->readData(data);
      update();
      emit readComplete();
   }
   catch (std::runtime_error e)
   {
      emit error(e.what());
   }

   if (read_again_when_finished && !terminate)
      readDataThread();
   else
      currently_reading = false;
}