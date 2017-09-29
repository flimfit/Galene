#pragma once

#include "AligningReader.h"
#include <string>
#include <memory>

#include <ome/files/VariantPixelBuffer.h>
#include <ome/files/in/OMETIFFReader.h>
#include "AbstractFrameAligner.h"

class IntensityReader : public AligningReader
{
public:

   IntensityReader(const std::string& filename);

   void getIntensityFrames() { read(); };

   ImageScanParameters getImageScanParameters() { return scan_params; }

protected:

   void readMetadata();
   void read();

   ImageScanParameters scan_params;
   std::shared_ptr<ome::files::FormatReader> reader;
   std::string filename;

   int n_t;
   int n_chan;
};
