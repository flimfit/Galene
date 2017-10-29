#include "IntensityReader.h"

class IntensityWriter
{
public:
   IntensityWriter(std::shared_ptr<IntensityReader> reader);
   void write(const std::string& filename);

protected:
   std::shared_ptr<IntensityReader> reader;
   int n_x, n_y, n_z, n_t, n_chan; 
};