#include "IntensityWriter.h"

class OmeIntensityWriter : public IntensityWriter
{
public:
   OmeIntensityWriter(std::shared_ptr<IntensityReader> reader);
   void write(const std::string& filename);
};