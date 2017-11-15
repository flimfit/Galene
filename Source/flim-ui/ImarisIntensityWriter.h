#include "IntensityWriter.h"
#include "ImarisIntensityReader.h"

class ImarisIntensityWriter : public IntensityWriter
{
public:
   ImarisIntensityWriter(std::shared_ptr<ImarisIntensityReader> reader);
   void write(const std::string& filename);
};
