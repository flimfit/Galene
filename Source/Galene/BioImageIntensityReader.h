#pragma once

#include "IntensityReader.h"
#include <bim_image_5d.h>

class BioImageIntensityReader : public IntensityReader
{
public:

   BioImageIntensityReader(const std::string& filename);
   static QStringList supportedExtensions();   

protected:

   void readMetadata();
   void addStack(int chan, int t, cv::Mat& data);

   std::shared_ptr<bim::Image5D> image5d;
};
