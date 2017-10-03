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

   void getIntensityFrames();

   ImageScanParameters getImageScanParameters() { return scan_params; }

   void read();
   void stopReading() { terminate = true; }
   void clearStopSignal() { terminate = false; }

   void write(const std::string& output_filename);

protected:

   void readMetadata();
   void readFrames(std::vector<cv::Mat>& data, bool merge_channels = false);
   void getRealignedStack(int chan, int t, cv::Mat& data);

   ImageScanParameters scan_params;
   std::shared_ptr<ome::files::FormatReader> reader;
   std::string filename;

   std::vector<cv::Mat> data;

   int n_x, n_y, n_z, n_t, n_chan;
   bool terminate = false;
};
