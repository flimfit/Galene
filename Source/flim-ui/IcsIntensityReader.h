#pragma once

#include "IntensityReader.h"
#include <libics.h>

struct DimensionOrder
{
   int x = 0;
   int y = 1;
   int z = 2;
   int c = 3;
   int t = 4;
};

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
   DimensionOrder dim_order;
};
