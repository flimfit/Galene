#pragma once

#include <QWidget>
#include <QString>
#include <QTimer>
#include <memory>
#include <thread>
#include <vector>
#include "TaskProgress.h"
#include "ThreadedObject.h"
#include "AligningReader.h"

class RealignableDataSource;

class DataSourceWorker : public ThreadedObject
{
   Q_OBJECT

public:
   DataSourceWorker(RealignableDataSource* source, QObject* parent = nullptr);

   Q_INVOKABLE void stop();
   void init();
   void update();
   
private:

   bool executing = false;
   RealignableDataSource* source;
   QTimer* timer;
};

class RealignableDataSource
{
public:
   
   void readData(bool realign = true);   
   void waitForComplete();
      
   const std::unique_ptr<AbstractFrameAligner>& getFrameAligner() { return aligningReader().getFrameAligner(); }
   void setReferenceIndex(int index) { aligningReader().setReferenceIndex(index); }
   void setRealignmentParameters(const RealignmentParameters& params) { aligningReader().setRealignmentParameters(params); }
   const std::vector<RealignmentResult>& getRealignmentResults() { return aligningReader().getRealignmentResults(); }
   
   virtual QString getFilename() = 0;
   virtual void requestDelete() = 0;
   virtual QWidget* getWidget() { return nullptr; }

protected:

   virtual AligningReader& aligningReader() = 0;
   
   virtual void update() = 0;
   virtual void setupForRead() = 0;
   virtual void alignFrames()  = 0;
   virtual void readAlignedData() = 0;

   // Use readData to call 
   void readDataThread(bool realign = true);   

   DataSourceWorker* worker;
   
   bool currently_reading = false;
   bool read_again_when_finished = false;
   bool terminate = false;
   std::thread reader_thread;
   
   std::shared_ptr<TaskProgress> task;
   
   friend class DataSourceWorker;   
};
