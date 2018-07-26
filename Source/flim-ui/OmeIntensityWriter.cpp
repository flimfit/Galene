#include "OmeIntensityWriter.h"

#include "ProducerConsumer.h"

IntensityWriter::IntensityWriter(std::shared_ptr<IntensityReader> reader, bool interpolate) :
reader(reader), interpolate(interpolate)
{
   n_x = reader->numX();
   n_y = reader->numY();
   n_z = reader->numZ();
   n_t = reader->numT();
   n_chan = reader->getNumChannels();
   cv_type = reader->getCvType();
}


OmeIntensityWriter::OmeIntensityWriter(std::shared_ptr<IntensityReader> reader, bool interpolate) :
   IntensityWriter(reader, interpolate)
{

}

void OmeIntensityWriter::write(const std::string& output_filename)
{
   using ome::xml::model::enums::PixelType;

   switch (cv_type)
   {
   case CV_8S: write_impl<PixelType::INT8>(output_filename); break;
   case CV_16S: write_impl<PixelType::INT16>(output_filename); break;
   case CV_32S: write_impl<PixelType::INT32>(output_filename); break;
   case CV_8U: write_impl<PixelType::UINT8>(output_filename); break;
   case CV_16U: write_impl<PixelType::UINT16>(output_filename); break;
   case CV_32F: write_impl<PixelType::FLOAT>(output_filename); break;
   case CV_64F: write_impl<PixelType::DOUBLE>(output_filename); break;
   case CV_32FC2: write_impl<PixelType::COMPLEXFLOAT>(output_filename); break;
   case CV_64FC2: write_impl<PixelType::COMPLEXDOUBLE>(output_filename); break;
   default: throw std::runtime_error("unsupported data format");
   }
}
