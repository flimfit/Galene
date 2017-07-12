#pragma once

#include "RealignmentStudio.h"
#include "ThreadedObject.h"
#include "TaskProgress.h"
#include <QStringList>

class RealignmentStudioBatchProcessor : public ThreadedObject
{
   Q_OBJECT

public:
   RealignmentStudioBatchProcessor(RealignmentStudio* studio_, QStringList files_)
   {
      studio = studio_;
      files = files_;

      task = std::make_shared<TaskProgress>("Processing...", true, files.size());
      TaskRegister::addTask(task);

      startThread();
   }

   void init()
   {
      QMetaObject::invokeMethod(this, "processNext", Qt::QueuedConnection);
   }

   Q_INVOKABLE 
   void processNext()
   {
      if (source)
      {
         studio->save(source, true);
         disconnect(source.get(), &FlimDataSource::readComplete, this, &RealignmentStudioBatchProcessor::processNext);
         task->incrementStep();
      }


      if (files.empty() || task->wasCancelRequested())
      {
         task->setFinished();
         deleteLater();
         return;
      }

      QString file = files.front();
      files.pop_front();

      source = studio->openFile(file);
      connect(source.get(), &FlimDataSource::readComplete, this, &RealignmentStudioBatchProcessor::processNext);
   }

private:

   std::shared_ptr<TaskProgress> task;
   std::shared_ptr<FlimReaderDataSource> source;
   RealignmentStudio* studio;
   QStringList files;
};
