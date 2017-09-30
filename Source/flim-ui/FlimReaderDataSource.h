#pragma once

#include "FlimDataSource.h"
#include "FlimReader.h"
#include "FlimCube.h"
#include "TaskProgress.h"
#include "RealignableDataSource.h"

#include <thread>
#include <memory>
#include <mutex>
#include <vector>

class FlimReaderDataSource : public FlimDataSource, public RealignableDataSource
{
   Q_OBJECT

signals:

   void error(const QString& message);
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

   void requestDelete() { emit deleteRequested(); }


   QString getFilename() { return QString::fromStdString(reader->getFilename()); }
   
   QWidget* getWidget();

   //   virtual std::list<std::vector<quint16>>& getHistogramData() = 0;
   std::vector<uint>& getCurrentDecay(int channel) { return current_decay_dummy; };
   std::vector<double>& getCountRates() { return count_rates_dummy; };
   std::vector<double>& getMaxInstantCountRates() { return count_rates_dummy; };

protected:

   void update();
   void setupForRead();
   void alignFrames();
   void readAlignedData();

   void cancelRead();

   AligningReader& aligningReader() { return *reader; }
   
   std::shared_ptr<FlimReader> reader;
   QString filename;

   std::vector<uint> current_decay_dummy;
   std::vector<double> count_rates_dummy;

   cv::Mat intensity;
   cv::Mat mean_arrival_time;

   std::shared_ptr<FlimCube<uint16_t>> data;

   std::mutex image_mutex;
   std::mutex read_mutex;
};