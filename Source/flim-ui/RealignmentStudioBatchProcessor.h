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
      QMetaObject::invokeMethod(this, "process", Qt::QueuedConnection);
   }

   Q_INVOKABLE 
   void process()
   {
      for(auto file : files)
      {
         auto source = studio->openFile(file);
         QThread::msleep(1000);
         source->readData(true);
         source->waitForComplete();
         studio->save(source, true);
         task->incrementStep();
      }

      task->setFinished();
      deleteLater();
   }

private:

   std::shared_ptr<TaskProgress> task;
   RealignmentStudio* studio;
   QStringList files;
};
