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


class RealignableDataOptions
{
public:

   void requestFromUserIfRequired(const AligningReader& reader);
   std::vector<bool> getChannelsToUse(size_t n_chan);

   bool getBidirectional() { return bidirectional; }
   int getNumZ() { return n_z; }

private:
   std::vector<bool> channels_to_use;
   bool bidirectional;
   int n_z;
   bool initalised = false;
};

class DataSourceWorker : public ThreadedObject
{
   Q_OBJECT

public:
   DataSourceWorker(RealignableDataSource* source, QObject* parent = nullptr);

   Q_INVOKABLE void stop();
   void init();
   void update();
   
signals:

   void readComplete();

private:

   bool executing = false;
   RealignableDataSource* source;
   QTimer* timer;
};

class RealignableDataSource
{

public:
   
   virtual ~RealignableDataSource();

   void readData(bool realign = true);   
   void waitForComplete();

   virtual void writeRealignmentMovies(const QString& filename_root); 
   void writeRealignmentInfo(const QString& filename_root);
   
   const std::unique_ptr<AbstractFrameAligner>& getFrameAligner() { return aligningReader().getFrameAligner(); }
   void setReferenceIndex(int index) { aligningReader().setReferenceIndex(index); }
   void setRealignmentParameters(const RealignmentParameters& params) { aligningReader().setRealignmentParameters(params); }
   const std::map<size_t,RealignmentResult>& getRealignmentResults() { return aligningReader().getRealignmentResults(); }
   virtual AligningReader& aligningReader() = 0;

   void setRealignmentOptions(RealignableDataOptions& options);

   virtual void savePreview(const QString& filename) {};
   virtual void saveData(const QString& filename, bool interpolate = false) = 0;   
   virtual QString getFilename() = 0;
   virtual void requestDelete() = 0;
   virtual QWidget* getWidget() { return nullptr; }

   DataSourceWorker* getWorker() { return worker; } // bit of a hack for now - we need to merge FlimDataSource and RealignableDataSource long term

   bool readIsComplete() { return read_is_complete; }

protected:

   
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
   bool read_is_complete = false;
   std::thread reader_thread;
   
   std::shared_ptr<TaskProgress> task;
   
   friend class DataSourceWorker;   
};
