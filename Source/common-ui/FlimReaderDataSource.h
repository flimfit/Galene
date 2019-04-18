#pragma once

#include "FlimDataSource.h"
#include "LiveFlimReader.h"
#include "FlimCube.h"
#include "TaskProgress.h"
#include "RealignableDataSource.h"

#include <thread>
#include <memory>
#include <mutex>
#include <vector>

class FlimDataSource : public RealignableDataSource
{
   Q_OBJECT

signals:

   void error(const QString& message);
   void deleteRequested(); 
   
public:

   FlimDataSource(const QString& filename, QObject* parent = 0);

   ~FlimDataSource();

   void init();

   std::shared_ptr<FlimReader> getReader() { return reader; }
   std::shared_ptr<FlimCube> getData() { return data; }

   void setForceNumZ(int n_z) { reader->setNumZ(n_z); }

   int getNumChannels() { return reader->getNumChannels(); }
   double getTimeResolution();

   cv::Mat getIntensity();
   cv::Mat getMeanArrivalTime();

   void requestDelete() { emit deleteRequested(); }


   QString getFilename() { return QString::fromStdString(reader->getFilename()); }
   
   void saveData(const QString& filename, bool interpolate = false);
   void savePreview(const QString& filename);
   
   QWidget* getWidget();

   //   virtual std::list<std::vector<quint16>>& getHistogramData() = 0;
   std::vector<uint>& getCurrentDecay(int channel) { return current_decay_dummy; };
   const std::vector<double>& getCountRates() { return reader->getCountRates(); };
//   std::vector<double>& getMaxInstantCountRates() { return count_rates; };

signals:
   void decayUpdated();
   void countRatesUpdated();
   void readComplete();

protected:

   FlimDataSource(QObject* parent = 0);


   void update();
   void setupForRead();
   void alignFrames();
   void readAlignedData();

   void cancelRead();

   std::shared_ptr<AligningReader> aligningReader() { return reader; }
   
   std::shared_ptr<FlimReader> reader;
   QString filename;

   std::vector<uint> current_decay_dummy;

   cv::Mat intensity;
   cv::Mat mean_arrival_time;

   std::shared_ptr<FlimCube> data;

   std::mutex image_mutex;
   std::mutex read_mutex;
};

