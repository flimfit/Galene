#pragma once

#include <QWidget>
#include <QString>
#include <QTimer>
#include <memory>
#include <future>
#include <vector>
#include "TaskProgress.h"
#include "ThreadedObject.h"
#include "AligningReader.h"

class RealignableDataSource;


class RealignableDataOptions
{
public:

   void requestFromUserIfRequired(std::shared_ptr<AligningReader> reader, QWidget* parent = nullptr);
   std::vector<bool> getChannelsToUse(size_t n_chan);

   bool getBidirectional() { return bidirectional; }
   int getNumZ() { return n_z; }

private:
   std::vector<bool> channels_to_use;
   bool bidirectional;
   int n_z;
   bool initalised = false;
};


class RealignableDataSource : public ThreadedObject
{
   Q_OBJECT 

public:
   
   RealignableDataSource(QObject* parent = 0);

   virtual ~RealignableDataSource();

   void readData(bool realign = true);   
   void waitForComplete();

   void init();
   Q_INVOKABLE void stop();
   void checkStatus();

   virtual void writeRealignmentMovies(const QString& filename_root); 
   void writeRealignmentInfo(const QString& filename_root);
   
   const std::unique_ptr<AbstractFrameAligner>& getFrameAligner() { return aligningReader()->getFrameAligner(); }
   cv::Mat getReferenceFrame() { return aligningReader()->getReferenceFrame(); }
   void setReferenceFrame(cv::Mat reference) { aligningReader()->setReferenceFrame(reference); }
   void setReferenceIndex(int index) { aligningReader()->setReferenceIndex(index); }
   void setRealignmentParameters(const RealignmentParameters& params) { aligningReader()->setRealignmentParameters(params); }
   const std::map<size_t,RealignmentResult>& getRealignmentResults() { return aligningReader()->getRealignmentResults(); }
   virtual std::shared_ptr<AligningReader> aligningReader() = 0;

   void setRealignmentOptions(RealignableDataOptions& options);

   virtual void savePreview(const QString& filename) {};
   virtual void saveData(const QString& filename, bool interpolate = false) = 0;   
   virtual QString getFilename() = 0;
   virtual void requestDelete() = 0;
   virtual QWidget* getWidget() { return nullptr; }

   bool readIsComplete() { return read_is_complete; }

signals:

   void readComplete();

protected:
   
   virtual void update() = 0;
   virtual void setupForRead() = 0;
   virtual void alignFrames()  = 0;
   virtual void readAlignedData() = 0;
   

   // Use readData to call 
   void readDataThread(bool realign = true);   

   bool executing = false;
   QTimer* timer;

   bool currently_reading = false;
   bool read_again_when_finished = false;
   bool terminate = false;
   bool read_is_complete = false;
   std::future<void> reader_thread;
   
   std::shared_ptr<TaskProgress> task;
   
   friend class DataSourceWorker;   
};
