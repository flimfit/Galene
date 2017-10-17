#include "RealignableDataSource.h"
#include "RealignmentResultsWriter.h"

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