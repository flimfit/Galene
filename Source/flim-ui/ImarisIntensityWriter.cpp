#include "ImarisIntensityWriter.h"
#include "IntensityReader.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "ProducerConsumer.h"
#include "Cv3dUtils.h"
#include <mutex>

extern std::mutex hdf5_mutex;

void pyrDown3d(const cv::Mat& m, cv::Mat& out)
{
   if (m.dims == 2) return pyrDown(m, out);

   int n_z_out = m.size[Z] / 2;
   std::vector<int> out_dims = { n_z_out, m.size[Y] / 2, m.size[X] / 2 };
   std::cout << n_z_out << ", " << m.size[Y] / 2 << ", " << m.size[X] / 2 << "\n";
   out = cv::Mat(out_dims, m.type());

   for(int z=0; z<n_z_out; z++)
      pyrDown(extractSlice(m, z*2), extractSlice(out, z));
}


ImarisIntensityWriter::ImarisIntensityWriter(std::shared_ptr<ImarisIntensityReader> reader) : 
   IntensityWriter(reader)
{

}

void ImarisIntensityWriter::write(const std::string& filename)
{
   std::unique_lock<std::mutex> lk(hdf5_mutex);
   boost::filesystem::copy_file(reader->getFilename(), filename, boost::filesystem::copy_option::overwrite_if_exists);

   // Create TIFF reader
   hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDWR | H5F_ACC_DEBUG, H5P_DEFAULT);
   if (file < 0)
      throw std::runtime_error("Error loading Imaris file");

   hid_t vDataSetId = H5Gopen(file, "DataSet", H5P_DEFAULT);

   hsize_t n_res_levels;
   H5Gget_num_objs(vDataSetId, &n_res_levels);

   std::vector<hid_t> levels;
   for(int i=0; i<n_res_levels; i++)
   {
      std::string res_level = "ResolutionLevel " + boost::lexical_cast<std::string>(i);
      levels.push_back(H5Gopen(vDataSetId, res_level.c_str(), H5P_DEFAULT));
   }

   lk.unlock();

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
      std::lock_guard<std::mutex> lk(hdf5_mutex);

      int chan = idx % n_chan;
      int t = idx / n_chan;

      std::string timepoint = "TimePoint " + boost::lexical_cast<std::string>(t);
      std::string channel = "Channel " + boost::lexical_cast<std::string>(chan);

      for(int i=0; i<levels.size(); i++)
      {
         hid_t vTimePointId = H5Gopen(levels[i], timepoint.data(), H5P_DEFAULT);
         hid_t vChannelId = H5Gopen(vTimePointId, channel.data(), H5P_DEFAULT);
         hid_t vDataId = H5Dopen(vChannelId, "Data", H5P_DEFAULT);
         hid_t type = H5Dget_type(vDataId);
         hid_t type_id = H5Tget_native_type(type, H5T_DIR_ASCEND);

         cvbuf.convertTo(cvbuf, getCvTypeFromH5T(type));  
         cv::Mat temp;       
         if (i > 0)
         {
            pyrDown3d(cvbuf, temp);
            cvbuf = temp;            
         }

         std::vector<uint64_t> dims = { (uint64_t) cvbuf.size[0], (uint64_t) cvbuf.size[1], (uint64_t) cvbuf.size[2] };
         hid_t vFileSpaceId = H5Screate_simple(3, dims.data(), NULL);
         H5Dwrite(vDataId, type_id, H5S_ALL, vFileSpaceId, H5P_DEFAULT, cvbuf.data);

         H5Gclose(vTimePointId);
         H5Gclose(vChannelId);
         H5Dclose(vDataId);
         H5Tclose(type);
         H5Tclose(type_id);
         H5Sclose(vFileSpaceId);
      }
   };

   int n_producer = 3; // to match cuda memory/kernel engines
   ProducerConsumer<cv::Mat>(n_producer, producer, consumer, n_t * n_chan);

   H5Gclose(vDataSetId);
   for(auto& l : levels)
      H5Gclose(l);
   H5Fclose(file);
}