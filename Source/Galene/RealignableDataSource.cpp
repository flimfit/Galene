#include "RealignableDataSource.h"
#include "RealignmentResultsWriter.h"
#include "CustomDialog.h"
#include <QSettings>
#include <QBitArray>

DataSourceWorker::DataSourceWorker(RealignableDataSource* source, QObject* parent) :
ThreadedObject(parent), source(source)
{
   startThread();
}

Q_INVOKABLE void DataSourceWorker::stop()
{
   timer->stop();
   bool executing = false;
}

void DataSourceWorker::init() 
{
   timer = new QTimer();
   connect(timer, &QTimer::timeout, this, &DataSourceWorker::update);
   connect(this, &QObject::destroyed, timer, &QObject::deleteLater);

   timer->setInterval(1000);
   timer->start();
   executing = true;
}

void DataSourceWorker::update()
{
   if (executing) 
      source->update();
   if (source->readIsComplete())
      emit readComplete();
}


RealignableDataSource::~RealignableDataSource()
{
   terminate = true;
   waitForComplete();
}

void RealignableDataSource::waitForComplete()
{
   if (reader_thread.joinable())
      reader_thread.join();
}

void RealignableDataSource::readData(bool realign)
{
   if (currently_reading)
   {
      //read_again_when_finished = true;
   }
   else
   {
      if (reader_thread.joinable())
         reader_thread.join();

      reader_thread = std::thread(&RealignableDataSource::readDataThread, this, realign);
   }
}


// Use readData to call 
void RealignableDataSource::readDataThread(bool realign)
{
   read_again_when_finished = false;
   currently_reading = true;
   read_is_complete = false;
   terminate = false;

   task = std::make_shared<TaskProgress>("Loading data...", true);
   TaskRegister::addTask(task);

   setupForRead();
   
   if (realign)
   {
      task->setTaskName("Preloading frames...");
      alignFrames();
   }
   task->setTaskName("Reading data...");
   readAlignedData();
   update();

   task->setFinished();

   if (read_again_when_finished && !terminate)
   {
      readDataThread();
   }
   else
   {
      currently_reading = false;
      read_is_complete = true;
   }
}

void RealignableDataSource::writeRealignmentMovies(const QString& filename_root)
{
   QString aligned_movie_filename = filename_root + "-aligned-stack.tif";
   QString aligned_preserving_movie_filename = filename_root + "-aligned-int-presv-stack.tif";
   QString unaligned_movie_filename = filename_root + "-unaligned-stack.tif";
   QString coverage_movie_filename = filename_root + "-coverage-stack.tif";
   QString info_filename = filename_root + "_realignment.csv";

   const auto& results = getRealignmentResults();
   RealignmentResultsWriter::exportAlignedMovie(results, aligned_movie_filename);
   RealignmentResultsWriter::exportAlignedIntensityPreservingMovie(results, aligned_preserving_movie_filename);
   RealignmentResultsWriter::exportUnalignedMovie(results, unaligned_movie_filename);
   RealignmentResultsWriter::exportCoverageMovie(results, coverage_movie_filename);

}

void RealignableDataSource::writeRealignmentInfo(const QString& filename_root)
{
   QString info_filename = filename_root + "_realignment.csv";
   
   if (getFrameAligner())
      getFrameAligner()->writeRealignmentInfo(info_filename.toStdString());   
}


void RealignableDataSource::setRealignmentOptions(RealignableDataOptions& options)
{
   auto reader = aligningReader();
   reader->setChannelsToUse(options.getChannelsToUse(reader->getNumChannels()));
   reader->setBidirectionalScan(options.getBidirectional());
   reader->setNumZ(options.getNumZ());
}

void RealignableDataOptions::requestFromUserIfRequired(std::shared_ptr<AligningReader> reader)
{
   int n_chan = reader->getNumChannels();
   bool request_bidirectional = !reader->canReadBidirectionalScan();
   bool request_num_z = !reader->canReadNumZ();

   if (!request_bidirectional)
      bidirectional = reader->getBidirectionalScan();
   if (!request_num_z)
      n_z = reader->getNumZ();

   if (initalised) return;

   QSettings settings;
   QBitArray last_use_chan = settings.value("realignable_data_source/last_use_chan", QBitArray()).toBitArray();

   if (n_chan == 1 && !request_num_z && !request_bidirectional) return;

   bool* use_chan = new bool[n_chan];

   CustomDialog d("Realignment Options", nullptr, BS_OKAY_ONLY);

   if (n_chan > 1)
   {
      d.addLabel("Please select channels to use for realignment");
      for (int i = 0; i<n_chan; i++)
      {
         use_chan[i] = (i < last_use_chan.size()) ? last_use_chan[i] : true;
         d.addCheckBox("Channel " + QString::number(i), &use_chan[i]);
      }
   }

   bidirectional = settings.value("realignable_data_source/last_bidirectional", false).toBool();
   n_z = settings.value("realignable_data_source/last_n_z", false).toInt();

   if (request_bidirectional | request_num_z)
   {
      d.addVSpacer(10);
      d.addLabel("Please specify the following scan parameters");
   }

   if (request_bidirectional)
      d.addCheckBox("Bidirectional scan?", &bidirectional);
   if (request_num_z)
      d.addSpinBox("Number of frames / stack", 1, 100000, &n_z, 1);

   d.exec();

   channels_to_use.resize(n_chan, true);
   if (n_chan > 1)
   {
      for (int i = 0; i<n_chan; i++)
         channels_to_use[i] = use_chan[i];

      last_use_chan.resize(std::max(last_use_chan.size(), n_chan));
      for (int i = 0; i<n_chan; i++)
         last_use_chan[i] = use_chan[i];
   }


   settings.setValue("realignable_data_source/last_use_chan", last_use_chan);
   settings.setValue("realignable_data_source/last_bidirectional", bidirectional);
   settings.setValue("realignable_data_source/last_n_z", n_z);

   delete[] use_chan;

   initalised = true;
}


std::vector<bool> RealignableDataOptions::getChannelsToUse(size_t n_chan)
{
   std::vector<bool> channels_to_use_out(n_chan, true);
   for (size_t i = 0; i < std::min(n_chan, channels_to_use.size()); i++)
      channels_to_use_out[i] = channels_to_use[i];
   return channels_to_use_out;
}