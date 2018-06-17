#pragma once

#include "IntensityReader.h"
#include <libics.h>

class IcsIntensityReader : public IntensityReader
{
public:

   IcsIntensityReader(const std::string& filename);
   ~IcsIntensityReader();

   static QStringList supportedExtensions();   

protected:

   void readMetadata();
   void addStack(int chan, int t, cv::Mat& data);

   ICS* ics = nullptr;
};
