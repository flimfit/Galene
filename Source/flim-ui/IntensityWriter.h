#include "IntensityReader.h"

class AbstractIntensityWriter
{
public:
   AbstractIntensityWriter(std::shared_ptr<IntensityReader> reader);
   virtual void write(const std::string& filename) = 0;

protected:
   std::shared_ptr<IntensityReader> reader;
   int n_x, n_y, n_z, n_t, n_chan;
};

class IntensityWriter : public AbstractIntensityWriter
{
public:
   IntensityWriter(std::shared_ptr<IntensityReader> reader);
   void write(const std::string& filename);
};