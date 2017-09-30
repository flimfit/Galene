#include "IntensityReader.h"

#include <Cv3dUtils.h>
#include <ome/files/out/OMETIFFWriter.h>


#include <ome/common/xml/Platform.h>
#include <ome/common/xml/dom/Document.h>

#include <ome/xml/Document.h>

#include <ome/xml/model/OME.h>
#include <ome/xml/model/OMEModel.h>
#include <ome/xml/model/detail/OMEModel.h>

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

   scan_params = ImageScanParameters(100, 100, 0, (int)reader->getSizeX(),
      (int)reader->getSizeY(),
      (int)reader->getSizeZ(), bidirectional);

   n_t = reader->getSizeT();
}

void IntensityReader::read()
{
   readFrames(data, false);
}

void IntensityReader::getIntensityFrames() 
{ 
   readFrames(frames, false); 
};


void IntensityReader::readFrames(std::vector<cv::Mat> data, bool merge_channels)
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
      reader->openBytes(p, buf);

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

void IntensityReader::write(const std::string& output_filename)
{
    ome::common::xml::dom::Document inputdoc(ome::xml::createDocument(filename));
    ome::xml::model::detail::OMEModel model;
    std::shared_ptr<ome::xml::model::OME> modelroot(std::make_shared<ome::xml::model::OME>());
    modelroot->update(inputdoc.getDocumentElement(), model);

   // Create TIFF writer
   auto writer = std::make_shared<ome::files::out::OMETIFFWriter>();

   auto meta = reader->getMetadataStore();

   // Set writer options before opening a file
   auto store = std::static_pointer_cast<ome::xml::meta::MetadataStore>(meta);
   writer->setMetadataStore(store);

   writer->setId(filename);
}


