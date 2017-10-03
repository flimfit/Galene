#include "IntensityDataSource.h"


IntensityDataSource::IntensityDataSource(const QString& filename, QObject* parent) : 
QObject(parent), filename(filename)
{
   reader = std::make_unique<IntensityReader>(filename.toStdString());
}


void IntensityDataSource::update()
{

}

void IntensityDataSource::setupForRead()
{
   connect(task.get(), &TaskProgress::cancelRequested, this, &IntensityDataSource::cancelRead);
   
}

void IntensityDataSource::alignFrames()
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

void IntensityDataSource::readAlignedData()
{
   try
   {
      reader->read(); // TODO      
   }
   catch (std::runtime_error e)
   {
      emit error(e.what());
   }
}

void IntensityDataSource::cancelRead()
{
   reader->stopReading(); 
   terminate = true;
}


void IntensityDataSource::saveData(const QString& filename)
{
   reader->write(filename.toStdString());
}