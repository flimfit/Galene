#pragma once

#include "RealignmentStudio.h"
#include "ThreadedObject.h"
#include "TaskProgress.h"
#include <QStringList>

class RealignmentStudioBatchProcessor : public QObject //: public ThreadedObject
{
   Q_OBJECT

public:
   RealignmentStudioBatchProcessor(RealignmentStudio* studio_, QStringList files_)
   {
      studio = studio_;
      files = files_;

      task = std::make_shared<TaskProgress>("Processing...", true, files.size());
      TaskRegister::addTask(task);

      processNext();
   }

   void processNext()
   {
      if (files.isEmpty() || task->wasCancelRequested())
      {
         source = nullptr;
         task->setFinished();
         return;
      }
      
      auto file = files.first();
      files.removeFirst();

      source = studio->openFileWithOptions(file, options);
      if (source)
      {
         connect(source->getWorker(), &DataSourceWorker::readComplete, this, &RealignmentStudioBatchProcessor::saveCurrent);
      }
      else
      {
         task->incrementStep();
         processNext();
      }
   }

   void saveCurrent()
   {
      disconnect(source->getWorker(), &DataSourceWorker::readComplete, this, &RealignmentStudioBatchProcessor::saveCurrent);
      studio->save(source, true);
      task->incrementStep();
      processNext();
   }

private:

   RealignableDataOptions options; // start uninitialised
   std::shared_ptr<RealignableDataSource> source;

   std::shared_ptr<TaskProgress> task;
   RealignmentStudio* studio;
   QStringList files;
};
