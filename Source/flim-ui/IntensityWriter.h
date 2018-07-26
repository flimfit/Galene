#pragma once

#include "IntensityReader.h"
#include <memory>
#include <string>

class IntensityWriter
{
public:
   IntensityWriter(std::shared_ptr<IntensityReader> reader, bool interpolate = false);
   virtual void write(const std::string& filename) = 0;

protected:
   std::shared_ptr<IntensityReader> reader;
   int n_x, n_y, n_z, n_t, n_chan, cv_type;
   bool interpolate;
};
