#include "IntensityReader.h"

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

IntensityReader::IntensityReader(const std::string& filename) :
      filename(filename)
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

void IntensityReader::readMetadata()
{
   // Get total number of images (series)
   auto ic = reader->getSeriesCount();

   auto a = reader->getCoreMetadataList();

   for (int channel = 0;
      channel < reader->getEffectiveSizeC();
      ++channel)
   {
   }


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


   n_t = std::min(n_t, 5);
   
   scan_params = ImageScanParameters(100, 100, 0, n_x, n_y, n_z, bidirectional);
}

void IntensityReader::read()
{
   waitForAlignmentComplete();
   readFrames(data, false);
}

void IntensityReader::getIntensityFrames() 
{ 
   readFrames(frames, false); 
};


void IntensityReader::readFrames(std::vector<cv::Mat>& data, bool merge_channels)
{
   // Get total number of planes (for this image index)
   int pc = reader->getImageCount();

   ome::files::VariantPixelBuffer buf;

   std::vector<int> dims = { scan_params.n_z, scan_params.n_y, scan_params.n_x };

   data.clear();
   for (int i = 0; i<n_t; i++)
      data.push_back(cv::Mat(dims, CV_32F, cv::Scalar(0)));

   cv::Mat cv32;

   // Loop over planes (for this image index)
   for (int p = 0; p < pc; ++p)
   {
      if (terminate) break;

      auto coords = reader->getZCTCoords(p);

      if (coords[2] < n_t)
      {
         reader->openBytes(p, buf);

         int nel = buf.num_elements();
         auto shape = buf.shape();

         // Create CV wrapper
         int type = getCvPixelType(buf.pixelType());
         cv::Mat cvbuf(scan_params.n_y, scan_params.n_x, type, buf.data());

         // Convert to 32F
         cvbuf.convertTo(cv32, CV_32F);

         // Copy into frame
         cv::Mat slice = extractSlice(data[coords[2]], coords[0]);
         slice += cv32;
      }
   }
}


void IntensityReader::write(const std::string& output_filename)
{
   using namespace ome::xml::meta;
   using namespace ome::xml::model;
   using namespace ome::files;

   // OME-XML metadata store.
   auto meta = std::make_shared<OMEXMLMetadata>();
   auto retrieve = std::static_pointer_cast<MetadataRetrieve>(meta);

   std::vector<std::shared_ptr<CoreMetadata>> series_list(1, std::make_shared<CoreMetadata>());
   auto core = series_list[0];
   core->sizeX = n_x;
   core->sizeY = n_y;
   core->sizeC = std::vector<dimension_size_type>(n_chan, 1);
   core->pixelType = enums::PixelType::UINT16;
   core->interleaved = false;
   core->bitsPerPixel = 16U;
   core->dimensionOrder = enums::DimensionOrder::XYZCT;
   fillMetadata(*meta, series_list);

   // Create TIFF writer
   auto writer = std::make_shared<out::OMETIFFWriter>();
   writer->setMetadataRetrieve(retrieve);
   writer->setId(output_filename);
   writer->setSeries(0);

   typedef PixelBuffer<PixelProperties<enums::PixelType::UINT16>::std_type> u16buffer;
   auto extents = boost::extents[n_x][n_y][n_z][1][1][1][1][1][1];
   auto storage_order = PixelBufferBase::make_storage_order(enums::DimensionOrder::XYZCT, false);

   std::vector<int> dims = {n_z, n_y, n_x};
   cv::Mat cvbuf(dims, CV_16U);

   // Loop over planes (for this image index)
   int idx = 0;
   for (int t = 0; t < n_t; t++)
      for (int c = 0; c < n_chan; c++)
      {
         getRealignedStack(c, t, cvbuf);

         for(int z = 0; z < n_z; z++)
         {
            auto zbuf = std::make_shared<u16buffer>(&cvbuf.at<uint16_t>(z, 0, 0),
               extents, enums::PixelType::UINT16, ENDIAN_NATIVE, storage_order);
            VariantPixelBuffer vbuf(zbuf);
            writer->saveBytes(idx++, vbuf);         
         }
      }

   writer->close();
}

void IntensityReader::getRealignedStack(int chan, int t, cv::Mat& data)
{
   frames[t].copyTo(data, CV_16U);
}
