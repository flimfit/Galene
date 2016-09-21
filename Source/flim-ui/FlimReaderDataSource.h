#pragma once

#include "FlimDataSource.h"
#include "FLIMReader.h"

#include <QString>
#include <memory>
#include <thread>
#include <mutex>

class FlimReaderDataSource : public FlimDataSource
{
public:

   FlimReaderDataSource(const QString& filename_, QObject* parent = 0) :
      FlimDataSource(parent)
   {
      timer = new QTimer(this);
      timer->setInterval(1000);
      connect(timer, &QTimer::timeout, this, &FlimReaderDataSource::update);
      filename = filename_;

      reader = std::shared_ptr<FLIMReader>(FLIMReader::createReader(filename.toStdString()));

      int n_chan = reader->getNumChannels();
      count_rates_dummy.resize(n_chan);

      reader->setTemporalResolution(0);

      readData();
   }

   std::shared_ptr<FLIMReader> getReader() { return reader; }

   int getNumChannels() { return reader->getNumChannels(); }
   double getTimeResolution() { return reader->getTemporalResolution(); };

   cv::Mat getIntensity() 
   { 
      std::lock_guard<std::mutex> lk(image_mutex);
      return intensity; 
   };

   cv::Mat getMeanArrivalTime() 
   { 
      std::lock_guard<std::mutex> lk(image_mutex);
      return intensity; 
   };

   //   virtual std::list<std::vector<quint16>>& getHistogramData() = 0;
   std::vector<uint>& getCurrentDecay(int channel) { return current_decay_dummy; };
   std::vector<double>& getCountRates() { return count_rates_dummy; };
   std::vector<double>& getMaxInstantCountRates() { return count_rates_dummy; };

protected:

   void readData()
   {
      timer->start();
      reader_thread = std::thread(&FlimReaderDataSource::readDataThread, this);
   }

   void update()
   {
      int n_px = reader->dataSizePerChannel();
      int n_chan = reader->getNumChannels();

      if (intensity.size().area() < n_px)
         return;

      uint16_t* data_ptr = data.data();
      for (int p = 0; p < n_px; p++)
      {
         uint16_t px = 0;
         for (int c = 0; c < n_chan; c++)
            px += *(data_ptr++);
         intensity.at<uint16_t>(p) = px;
      }

      emit decayUpdated();
   }
   
   // Use readData to call 
   void readDataThread()
   {
      int n_px = reader->dataSizePerChannel();
      int n_chan = reader->getNumChannels();
      int n_x = reader->numX();
      int n_y = reader->numY();

      {
         std::lock_guard<std::mutex> lk(image_mutex);
         intensity = cv::Mat(n_y, n_x, CV_16U, cv::Scalar(0));
      }

      std::vector<int> channels(n_chan);
      for (int i = 0; i < n_chan; i++)
         channels[i] = i;

      data.resize(n_px *  n_chan);
      reader->readData(data.data(), channels);
   }

   std::shared_ptr<FLIMReader> reader;
   QString filename;

   std::vector<uint> current_decay_dummy;
   std::vector<double> count_rates_dummy;

   cv::Mat intensity;
   cv::Mat mean_arrival_time;

   std::vector<uint16_t> data;

   std::thread reader_thread;
   std::mutex image_mutex;

   QTimer* timer;
};