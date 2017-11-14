#include "IntensityDataSource.h"
#include "ImarisIntensityWriter.h"
#include "OmeIntensityWriter.h"

IntensityDataSource::IntensityDataSource(const QString& filename, QObject* parent) : 
QObject(parent), filename(filename)
{
   reader = IntensityReader::getReader(filename.toStdString());

   worker = new DataSourceWorker(this);
   connect(this, &QObject::destroyed, worker, &QObject::deleteLater);
}


void IntensityDataSource::update()
{
   if (terminate)
      return;

   if (task)
   {
      double progress = reader->getProgress();
      task->setProgress(progress);
   }
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
      reader->waitForAlignmentComplete();
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


void IntensityDataSource::saveData(const QString& root_name)
{
   auto imaris_writer = std::dynamic_pointer_cast<ImarisIntensityReader>(reader);

   if (imaris_writer)
   {
      QString filename = root_name + ".ims";
      ImarisIntensityWriter writer(imaris_writer);
      writer.write(filename.toStdString());
   }
   else
   {
      QString filename = root_name + ".ome.tif";
      OmeIntensityWriter writer(reader);
      writer.write(filename.toStdString());
   }
}