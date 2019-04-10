#pragma once

#include "AligningReader.h"
#include <string>
#include <memory>
#include <QStringList>

#include "AbstractFrameAligner.h"

class IntensityReader : public AligningReader
{
public:

   IntensityReader(const std::string& filename);
   ~IntensityReader() { }

   static std::shared_ptr<IntensityReader> getReader(const std::string& filename);
   const std::string& getFilename() { return filename; }
   int getNumIntensityFrames() { return n_t; };
   ImageScanParameters getImageScanParameters() { return scan_params; }

   void read();
   void stopReading() { terminate = true; }
   void clearStopSignal() { terminate = false; }

   int numX() { return n_x; }
   int numY() { return n_y; }
   int numZ() { return n_z; }
   int numT() { return n_t; }
   int getNumChannels() const { return n_chan; }
   int getCvType() { return cv_type; }
   
   cv::Mat getRealignedStack(int chan, int t, bool interpolate);

   virtual bool canReadBidirectionalScan() const { return false; }
   void setBidirectionalScan(bool bidirectional = true) { scan_params.bidirectional = bidirectional;  };
   bool getBidirectionalScan() const { return scan_params.bidirectional; }

   bool canReadNumZ() const { return true; }
   int getNumZ() const { return n_z; }

   double getProgress();
   
   static QStringList supportedExtensions();

   cv::Mat getStack(int chan, int t);

protected:

   virtual void addStack(int chan, int t, cv::Mat& data) = 0;

   void loadIntensityFramesImpl();
   cv::Mat getIntensityFrameImmediately(int frame);

   ImageScanParameters scan_params;
   std::string filename;

   std::vector<cv::Mat> data;

   std::mutex read_mutex;

   int n_x, n_y, n_z, n_t, n_chan, cv_type;

   bool swap_zt = false;
};
