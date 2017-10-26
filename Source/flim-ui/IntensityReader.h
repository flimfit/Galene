#pragma once

#include "AligningReader.h"
#include <string>
#include <memory>

#include <ome/files/VariantPixelBuffer.h>
#include <ome/files/in/OMETIFFReader.h>
#include "AbstractFrameAligner.h"

class IntensityReader : public AligningReader
{
public:

   IntensityReader(const std::string& filename);

   int getNumIntensityFrames() { return n_t; };

   ImageScanParameters getImageScanParameters() { return scan_params; }

   void read();
   void stopReading() { terminate = true; }
   void clearStopSignal() { terminate = false; }

   void write(const std::string& output_filename);

protected:

   void readMetadata();
   void loadIntensityFramesImpl();
   cv::Mat getIntensityFrameImmediately(int frame);


   cv::Mat getStack(int chan, int t);
   cv::Mat getRealignedStack(int chan, int t);

   void addStack(int chan, int t, cv::Mat& data);

   ImageScanParameters scan_params;
   std::shared_ptr<ome::files::FormatReader> reader;
   std::string filename;

   std::vector<cv::Mat> data;

   std::mutex read_mutex;

   int n_x, n_y, n_z, n_t, n_chan;
   bool terminate = false;
};
