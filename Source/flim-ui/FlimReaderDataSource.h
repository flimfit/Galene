#pragma once

#include "FlimDataSource.h"
#include "FlimReader.h"
#include "FlimCube.h"
#include "TaskProgress.h"
#include "RealignableDataSource.h"

#include <QString>
#include <QThread>
#include <QTimer>
#include "ThreadedObject.h"

#include <thread>
#include <memory>
#include <mutex>
#include <vector>

class FlimReaderDataSource;

class FlimReaderDataSourceWorker : public ThreadedObject
{
   Q_OBJECT

public:
   FlimReaderDataSourceWorker(QObject* parent, FlimReaderDataSource* source) :
      ThreadedObject(parent), source(source)
   {
      startThread();
   }

   Q_INVOKABLE void stop()
   {
      timer->stop();
      bool executing = false;
   }

   void init() 
   {
      timer = new QTimer();
      connect(timer, &QTimer::timeout, this, &FlimReaderDataSourceWorker::update);
      connect(this, &QObject::destroyed, timer, &QObject::deleteLater);

      timer->setInterval(1000);
      timer->start();
      executing = true;
   }

   void update();

signals:

   void updateComplete();

private:

   bool executing = false;
   FlimReaderDataSource* source;
   QTimer* timer;
};

class FlimReaderDataSource : public FlimDataSource, public RealignableDataSource
{
   Q_OBJECT

signals:

   void error(const QString& message);
   void alignmentComplete();
   void deleteRequested(); 
   
public:

   FlimReaderDataSource(const QString& filename_, QObject* parent = 0);
   ~FlimReaderDataSource();

   std::shared_ptr<FlimReader> getReader() { return reader; }
   std::shared_ptr<FlimCube<uint16_t>> getData() { return data; }

   int getNumChannels() { return reader->getNumChannels(); }
   double getTimeResolution() { return reader->getTemporalResolution(); };

   cv::Mat getIntensity();
   cv::Mat getMeanArrivalTime();

   void readData(bool realign = true);
   void waitForComplete();
   void requestDelete() { emit deleteRequested(); }


   void setRealignmentParameters(const RealignmentParameters& params) { reader->setRealignmentParameters(params); }
   const std::unique_ptr<AbstractFrameAligner>& getFrameAligner() { return reader->getFrameAligner(); }
   QString getFilename() { return QString::fromStdString(reader->getFilename()); }
   void setReferenceIndex(int index) { reader->setReferenceIndex(index); }
   const std::vector<RealignmentResult>& getRealignmentResults() { return reader->getRealignmentResults(); }
   
   QWidget* getWidget();

   //   virtual std::list<std::vector<quint16>>& getHistogramData() = 0;
   std::vector<uint>& getCurrentDecay(int channel) { return current_decay_dummy; };
   std::vector<double>& getCountRates() { return count_rates_dummy; };
   std::vector<double>& getMaxInstantCountRates() { return count_rates_dummy; };

protected:

   void update();
   
   // Use readData to call 
   void readDataThread(bool realign = true);

   std::shared_ptr<FlimReader> reader;
   QString filename;

   std::vector<uint> current_decay_dummy;
   std::vector<double> count_rates_dummy;

   cv::Mat intensity;
   cv::Mat mean_arrival_time;

   std::shared_ptr<FlimCube<uint16_t>> data;

   std::thread reader_thread;
   std::mutex image_mutex;
   std::mutex read_mutex;

   FlimReaderDataSourceWorker* worker;

   bool currently_reading = false;
   bool read_again_when_finished = false;
   bool terminate = false;

   std::shared_ptr<TaskProgress> task;

   friend class FlimReaderDataSourceWorker;
};