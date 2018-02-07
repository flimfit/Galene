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
      readDataThread();
   else
      currently_reading = false;
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

void RealignableDataSource::requestChannelsFromUser()
{
   int n_chan = aligningReader().getNumChannels();
   bool request_bidirectional = !aligningReader().canReadBidirectionalScan();
   bool request_num_z = !aligningReader().canReadNumZ(); 

   QSettings settings;
   QBitArray last_use_chan = settings.value("realignable_data_source/last_use_chan", QBitArray()).toBitArray();

   if (n_chan == 1 && !request_num_z) return;

   bool* use_chan = new bool[n_chan];

   CustomDialog d("Realignment Options", nullptr, BS_OKAY_ONLY);
   d.addLabel    ("Please select channels to use for realignment");
   for(int i=0; i<n_chan; i++)
   {
      use_chan[i] = (i < last_use_chan.size()) ? last_use_chan[i] : true;
      d.addCheckBox("Channel " + QString::number(i), &use_chan[i]);
   }


   bool bidirectional = settings.value("realignable_data_source/last_bidirectional", false).toBool();
   int n_z = settings.value("realignable_data_source/last_n_z", false).toInt();

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

   std::vector<bool> use_chan_v(n_chan);
   for(int i=0; i<n_chan; i++)
      use_chan_v[i] = use_chan[i];

   aligningReader().setChannelsToUse(use_chan_v);

   if (request_bidirectional)
      aligningReader().setBidirectionalScan(bidirectional);
   if (request_num_z)
      aligningReader().setNumZ(n_z);

   last_use_chan.resize(std::max(last_use_chan.size(), n_chan));
   for(int i=0; i<n_chan; i++)
      last_use_chan[i] = use_chan[i];

   settings.setValue("realignable_data_source/last_use_chan", last_use_chan);
   settings.setValue("realignable_data_source/last_bidirectional", bidirectional);
   settings.setValue("realignable_data_source/last_n_z", n_z);

   delete[] use_chan;   
}
