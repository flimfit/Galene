#include "IntensityReader.h"

#include <ome/files/in/OMETIFFReader.h>

class OmeIntensityReader : public IntensityReader
{
public:
   OmeIntensityReader(const std::string& filename);
   static QStringList supportedExtensions();
   
protected:

   
   void readMetadata();
   void addStack(int chan, int t, cv::Mat& data);

   std::shared_ptr<ome::files::FormatReader> reader; 
};