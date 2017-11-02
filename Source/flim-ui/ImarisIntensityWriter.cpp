#include "ImarisIntensityWriter.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "ProducerConsumer.h"

ImarisIntensityWriter::ImarisIntensityWriter(std::shared_ptr<ImarisIntensityReader> reader) : 
   AbstractIntensityWriter(reader)
{

}

void ImarisIntensityWriter::write(const std::string& filename)
{
   boost::filesystem::copy_file(reader->getFilename(), filename, boost::filesystem::copy_option::overwrite_if_exists);

   // Create TIFF reader
   hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY | H5F_ACC_DEBUG, H5P_DEFAULT);
   if (file < 0)
      throw std::runtime_error("Error loading Imaris file");

   hid_t vDataSetId = H5Gopen(file, "DataSet", H5P_DEFAULT);
   hid_t level0 = H5Gopen(vDataSetId, "ResolutionLevel 0", H5P_DEFAULT);



   size_t dims[2] = {n_y, n_x};

   auto producer = [&](size_t idx)
   {
      int chan = idx % n_chan;
      int t = idx / n_chan;

      cv::Mat cvbuf;
      cv::Mat stack = reader->getRealignedStack(chan, t);
      stack.convertTo(cvbuf, CV_8U);
      return cvbuf;
   };

   auto consumer = [&](size_t idx, cv::Mat cvbuf)
   {
      int chan = idx % n_chan;
      int t = idx / n_chan;

      std::string timepoint = "TimePoint " + boost::lexical_cast<std::string>(t);
      std::string channel = "Channel " + boost::lexical_cast<std::string>(chan);

      hid_t vTimePointId = H5Gopen(level0, timepoint.data(), H5P_DEFAULT);
      hid_t vChannelId = H5Gopen(vTimePointId, channel.data(), H5P_DEFAULT);
      hid_t vDataId = H5Dopen(vChannelId, "Data", H5P_DEFAULT);
      hid_t type = H5Dget_type(vDataId);
      hid_t type_id = H5Tget_native_type(type, H5T_DIR_ASCEND);

      int cv_type = -1;
      if (H5Tequal(type, H5T_NATIVE_UCHAR))
         cv_type = CV_8U;
      else if (H5Tequal(type, H5T_NATIVE_USHORT))
         cv_type = CV_16U;
      else if (H5Tequal(type, H5T_NATIVE_UINT32))
         cv_type = CV_32S;
      else if (H5Tequal(type, H5T_NATIVE_FLOAT))
         cv_type = CV_32F;


      std::vector<uint64_t> dims = { (uint64_t)n_z, (uint64_t)n_y, (uint64_t)n_x };
      hid_t vFileSpaceId = H5Screate_simple(3, dims.data(), NULL);

      std::vector<int> cv_dims = { n_z, n_y, n_x };
      cv::Mat cv2;
      cvbuf.convertTo(cv2, cv_type);
      
      H5Dwrite(vDataId, type_id, H5S_ALL, vFileSpaceId, H5P_DEFAULT, cv2.data);

      H5Gclose(vTimePointId);
      H5Gclose(vChannelId);
      H5Dclose(vDataId);
      H5Tclose(type);
      H5Tclose(type_id);
      H5Sclose(vFileSpaceId);
   };

   int n_producer = 3; // to match cuda memory/kernel engines
   ProducerConsumer<cv::Mat>(n_producer, producer, consumer, n_t * n_chan);

   H5Gclose(vDataSetId);
   H5Gclose(level0);
   H5Fclose(file);
}