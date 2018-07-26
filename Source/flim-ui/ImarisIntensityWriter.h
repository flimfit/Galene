#include "IntensityWriter.h"
#include "ImarisIntensityReader.h"

class ImarisIntensityWriter : public IntensityWriter
{
public:
   ImarisIntensityWriter(std::shared_ptr<ImarisIntensityReader> reader, bool interpolate = false);
   void write(const std::string& filename);
};
