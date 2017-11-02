#include "IntensityWriter.h"
#include "ImarisIntensityReader.h"

class ImarisIntensityWriter : public AbstractIntensityWriter
{
public:
   ImarisIntensityWriter(std::shared_ptr<ImarisIntensityReader> reader);
   void write(const std::string& filename);
};
