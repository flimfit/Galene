#include "RealignableDataSource.h"

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
