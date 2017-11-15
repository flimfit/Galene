#include "OmeIntensityReader.h"

#include <Cv3dUtils.h>
#include <ome/files/CoreMetadata.h>
#include <ome/files/MetadataTools.h>
#include <ome/files/VariantPixelBuffer.h>
#include <ome/files/out/OMETIFFWriter.h>
#include <ome/xml/meta/OMEXMLMetadata.h>

int getCvPixelType(ome::xml::model::enums::PixelType pixel_type)
{
   switch (pixel_type)
   {
   case ome::xml::model::enums::PixelType::INT8: return CV_8S;
   case ome::xml::model::enums::PixelType::INT16: return CV_16S;
   case ome::xml::model::enums::PixelType::INT32: return CV_32S;
   case ome::xml::model::enums::PixelType::UINT8: return CV_8U;
   case ome::xml::model::enums::PixelType::UINT16: return CV_16U;
   case ome::xml::model::enums::PixelType::UINT32: return CV_32S; // OpenCV doesn't have CV_32U type
   case ome::xml::model::enums::PixelType::FLOAT: return CV_32F;
   case ome::xml::model::enums::PixelType::DOUBLE: return CV_64F;
   case ome::xml::model::enums::PixelType::COMPLEXFLOAT: return CV_32FC2;
   case ome::xml::model::enums::PixelType::COMPLEXDOUBLE: return CV_64FC2;
   default: throw std::runtime_error("unsupported data format");
   }
   return CV_8S;
}

OmeIntensityReader::OmeIntensityReader(const std::string& filename) :
   IntensityReader(filename)
{
   // Create TIFF reader
   reader = std::make_shared<ome::files::in::OMETIFFReader>();

   // Set reader options before opening a file
   reader->setMetadataFiltered(false);
   reader->setGroupFiles(true);

   // Open the file
   reader->setId(filename);

   readMetadata();
}


QStringList OmeIntensityReader::supportedExtensions()
{
   return { "ome.tif", "ome.tiff", "ome.btf" };
}

void OmeIntensityReader::readMetadata()
{
   // Get total number of images (series)
   auto ic = reader->getSeriesCount();

   if (ic > 1)
      std::cout << "Only reading first image";

   // Change the current series to this index
   reader->setSeries(0);

   bool bidirectional = false;

   n_x = (int)reader->getSizeX();
   n_y = (int)reader->getSizeY();
   n_z = (int)reader->getSizeZ();
   n_t = (int)reader->getSizeT();
   n_chan = (int)reader->getSizeC();

   swap_zt = (n_t == 1 && n_z > 1);
   if (swap_zt)
   {
      int nb = n_t;
      n_t = n_z;
      n_z = nb;
      std::cout << "Swapping z and t\n";
   }

   n_t--; // TODO: Last frame seems problematic 

   setUseAllChannels();
   
   scan_params = ImageScanParameters(100, 100, 0, n_x, n_y, n_z, bidirectional);
}

void OmeIntensityReader::addStack(int chan, int t, cv::Mat& data)
{
   std::lock_guard<std::mutex> lk(read_mutex);
   ome::files::VariantPixelBuffer buf;
   cv::Mat cv16;

   for (int z = 0; z < n_z; z++)
   {
      try
      {
         size_t index = swap_zt ? reader->getIndex(t, chan, z) : reader->getIndex(z, chan, t);
         reader->openBytes(index, buf);

         int type = getCvPixelType(buf.pixelType());
         cv::Mat cvbuf(scan_params.n_y, scan_params.n_x, type, buf.data());
         cvbuf.convertTo(cv16, CV_16U);

         // Copy into frame
         extractSlice(data, z) += cv16;
      }
      catch (std::exception e)
      {
         std::cout << e.what();
      }
   }
}