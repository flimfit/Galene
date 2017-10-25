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
   
   n_t = 3; // TODO: Last frame seems problematic 

   scan_params = ImageScanParameters(100, 100, 0, n_x, n_y, n_z, bidirectional);
}

void IntensityReader::read()
{
   waitForAlignmentComplete();
}

void IntensityReader::addStack(int chan, int t, cv::Mat& data)
{
   std::lock_guard<std::mutex> lk(read_mutex);
   ome::files::VariantPixelBuffer buf;
   cv::Mat cv8;

   for (int z = 0; z < n_z; z++)
   {
      try
      {
         auto index = reader->getIndex(z, chan, t);
         reader->openBytes(index, buf);

         int type = getCvPixelType(buf.pixelType());
         cv::Mat cvbuf(scan_params.n_y, scan_params.n_x, type, buf.data());
         cvbuf.convertTo(cv8, CV_8U);

         // Copy into frame
         extractSlice(data, z) += cv8;
      }
      catch (std::exception e)
      {
         std::cout << e.what();
      }
   }
}

cv::Mat IntensityReader::getStack(int chan, int t)
{
   std::vector<int> dims = { n_z, n_y, n_x };
   cv::Mat stack(dims, CV_8U, cv::Scalar(0));
   addStack(chan, t, stack);
   return stack;
}


void IntensityReader::loadIntensityFramesImpl()
{
   std::vector<int> dims = { n_z, n_y, n_x };

   cv::Mat cur_frame(dims, CV_8U, cv::Scalar(0));

   {
      std::lock_guard<std::mutex> lk(frame_mutex);
      frames.resize(n_t);
   }

   // Loop over planes (for this image index)
   for (int t = 0; t < n_t; ++t)
   {
      cur_frame.setTo(cv::Scalar(0));
      if (terminate) break;
       for(int chan = 0; chan < n_chan; chan++)
         addStack(chan, t, cur_frame);

       cv::Mat frame_cpy;
       cur_frame.copyTo(frame_cpy);
       
       {
          std::lock_guard<std::mutex> lk(frame_mutex);
          frames[t] = frame_cpy;
       }
       frame_cv.notify_all();

   }
}


void IntensityReader::write(const std::string& output_filename)
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
   writer->setCompression("LZW");
   writer->setSeries(0);
   typedef PixelBuffer<PixelProperties<pixel_type>::std_type> pxbuffer;
   auto extents = boost::extents[n_x][n_y][1][1][1][1][1][1][1];
   auto storage_order = PixelBufferBase::make_storage_order(dim_order, false);

   auto series_metadata = reader->getGlobalMetadata();
   
   auto store = reader->getMetadataStore();

   for (auto& s : series_metadata)
   {
      auto a1 = s.first;
      auto s2 = s.second;
      std::cout << a1 << " : " << s2 << "\n";
   }


   VariantPixelBuffer lut;
   cv::Mat cvbuf;

   // Loop over planes (for this image index)
   int idx = 0;
   for (int t = 0; t < n_t; t++)
      for (int c = 0; c < n_chan; c++)
      {
         cv::Mat stack = getRealignedStack(c, t);
         stack.convertTo(cvbuf, CV_8U);

         for(int z = 0; z < n_z; z++)
         {
            auto zbuf = std::make_shared<pxbuffer>(&cvbuf.at<uint8_t>(z, 0, 0),
               extents, pixel_type, ENDIAN_NATIVE, storage_order);
            VariantPixelBuffer vbuf(zbuf);
            writer->saveBytes(idx++, vbuf);
         }
      }

   writer->close();
}

cv::Mat IntensityReader::getRealignedStack(int chan, int t)
{
   cv::Mat realigned;
   cv::Mat stack = getStack(chan, t);
   stack.convertTo(stack, CV_32F);
   
   if (frame_aligner)
      realigned = frame_aligner->realignAsFrame(t, stack);
   else
      realigned = stack;

   return realigned;
}
