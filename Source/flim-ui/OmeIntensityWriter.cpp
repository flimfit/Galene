#include "OmeIntensityWriter.h"

#include <ome/files/CoreMetadata.h>
#include <ome/files/MetadataTools.h>
#include <ome/files/VariantPixelBuffer.h>
#include <ome/files/out/OMETIFFWriter.h>
#include <ome/xml/meta/OMEXMLMetadata.h>

#include "ProducerConsumer.h"

IntensityWriter::IntensityWriter(std::shared_ptr<IntensityReader> reader) :
reader(reader)
{
   n_x = reader->numX();
   n_y = reader->numY();
   n_z = reader->numZ();
   n_t = reader->numT();
   n_chan = reader->numC();
}


OmeIntensityWriter::OmeIntensityWriter(std::shared_ptr<IntensityReader> reader) : 
   IntensityWriter(reader)
{
}

void OmeIntensityWriter::write(const std::string& output_filename)
{
   using namespace ome::xml::meta;
   using namespace ome::xml::model;
   using namespace ome::files;

   // OME-XML metadata store.
   auto meta = std::make_shared<OMEXMLMetadata>();

   const auto pixel_type = enums::PixelType::UINT8;
   const auto dim_order = enums::DimensionOrder::XYZTC;
   const auto file_dim_order = enums::DimensionOrder::XYZCT;

   std::vector<std::shared_ptr<CoreMetadata>> series_list;
   auto core = std::make_shared<CoreMetadata>();
   core->sizeX = n_x;
   core->sizeY = n_y;
   core->sizeZ = n_z;
   core->sizeT = n_t;
   core->sizeC = std::vector<dimension_size_type>(n_chan, 1);
   core->pixelType = pixel_type;
   core->interleaved = false;
   core->imageCount = n_z * n_t * n_chan;
   core->bitsPerPixel = PixelProperties<pixel_type>::pixel_bit_size();
   core->dimensionOrder = file_dim_order;
   series_list.push_back(core);
   fillMetadata(*meta, series_list);

   auto a = meta->getPixelsDimensionOrder(0);

   // Create TIFF writer
   auto writer = std::make_shared<out::OMETIFFWriter>();
   auto retrieve = std::static_pointer_cast<MetadataRetrieve>(meta);
   
   writer->setMetadataRetrieve(retrieve);
   writer->setInterleaved(false);
   writer->setWriteSequentially(true);
   writer->setId(output_filename);
   writer->setSeries(0);
   auto order = writer->getDimensionOrder();
   typedef PixelBuffer<PixelProperties<pixel_type>::std_type> pxbuffer;
   auto extents = boost::extents[n_x][n_y][1][1][1][1][1][1][1];
   auto storage_order = PixelBufferBase::make_storage_order(dim_order, false);

   VariantPixelBuffer lut;

   auto producer = [&](size_t idx)
   {
      int c = idx % n_chan;
      int t = idx / n_chan;

      cv::Mat cvbuf;
      cv::Mat stack = reader->getRealignedStack(c, t);
      stack.convertTo(cvbuf, CV_8U);
      return cvbuf;
   };

   auto consumer = [&](size_t idx, cv::Mat cvbuf)
   {
      for (int z = 0; z < n_z; z++)
      {
         auto zbuf = std::make_shared<pxbuffer>(&cvbuf.at<uint8_t>(z, 0, 0),
            extents, pixel_type, ENDIAN_NATIVE, storage_order);
         VariantPixelBuffer vbuf(zbuf);
         writer->saveBytes(idx*n_z + z, vbuf);
      }
   };

   int n_producer = 3; // to match cuda memory/kernel engines
   ProducerConsumer<cv::Mat>(n_producer, producer, consumer, n_t * n_chan);

   writer->close();
}
