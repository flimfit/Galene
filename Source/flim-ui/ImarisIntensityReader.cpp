#include "ImarisIntensityReader.h"
#include <boost/lexical_cast.hpp>
#include <Cv3dUtils.h>

using namespace H5;

template<typename T>
T readAttribute(hid_t h, const std::string& attr)
{
   // open attribute LSMEmissionWavelength
   hid_t vAttributeId = H5Aopen_name(h, attr.c_str());
   hid_t vAttributeSpaceId = H5Aget_space(vAttributeId);
   hsize_t vAttributeSize = 0;
   H5Sget_simple_extent_dims(vAttributeSpaceId, &vAttributeSize, NULL);
   std::string vBuffer(vAttributeSize, 0);
   H5Aread(vAttributeId, H5T_C_S1, &vBuffer[0]);
   std::cout << "Attr: " << attr << " = " << vBuffer << "\n";
   return boost::lexical_cast<T>(vBuffer);
}

ImarisIntensityReader::ImarisIntensityReader(const std::string& filename) :
   IntensityReader(filename)
{
   // Create TIFF reader
   file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY | H5F_ACC_DEBUG, H5P_DEFAULT);
   if (file < 0)
      throw std::runtime_error("Error loading Imaris file");

   hid_t vDataSetId = H5Gopen(file, "DataSet", H5P_DEFAULT);
   level0 = H5Gopen(vDataSetId, "ResolutionLevel 0", H5P_DEFAULT);

   readMetadata();
}


QStringList ImarisIntensityReader::supportedExtensions()
{
   return { "ims" };
}

void ImarisIntensityReader::readMetadata()
{
   hid_t vDataSetInfoId = H5Gopen(file, "DataSetInfo", H5P_DEFAULT);
   hid_t vImageId = H5Gopen(vDataSetInfoId, "Image", H5P_DEFAULT);

   n_x = readAttribute<int>(vImageId, "X");
   n_y = readAttribute<int>(vImageId, "Y");
   n_z = readAttribute<int>(vImageId, "Z");
   H5Gclose(vImageId);

   n_chan = 0;
   hid_t h = -1;
   do
   {
      std::string name = "Channel " + boost::lexical_cast<std::string>(++n_chan);
      h = H5Gopen(vDataSetInfoId, name.c_str(), H5P_DEFAULT);
      if (h >= 0) H5Gclose(h);
      std::cout << h << "\n";
   } while (h >= 0);
   n_chan--;
 
   hid_t vTimeInfo = H5Gopen(vDataSetInfoId, "TimeInfo", H5P_DEFAULT);
   n_t = readAttribute<int>(vTimeInfo, "FileTimePoints");
   H5Gclose(vTimeInfo);

   H5Gclose(vDataSetInfoId);
   
   bool bidirectional = false;
   scan_params = ImageScanParameters(100, 100, 0, n_x, n_y, n_z, bidirectional);
}

void ImarisIntensityReader::addStack(int chan, int t, cv::Mat& data)
{
   std::lock_guard<std::mutex> lk(read_mutex);
   
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


   std::vector<uint64_t> dims = {(uint64_t) n_z, (uint64_t) n_y, (uint64_t) n_x};
   hid_t vFileSpaceId = H5Screate_simple(3, dims.data(), NULL);

   //std::cout << "cvtype " << cv_type << ", " << n_z << "," << n_y << "," << n_x << "\n";
   

   std::vector<int> cv_dims = {n_z, n_y, n_x};
   cv::Mat buf(cv_dims, cv_type);
   H5Dread(vDataId, H5T_NATIVE_UCHAR, H5S_ALL, vFileSpaceId, H5P_DEFAULT, buf.data);

   cv::Mat cv16;
   buf.convertTo(cv16, CV_16U);
   data += cv16;

   H5Tclose(type_id);
   H5Sclose(vFileSpaceId);
   H5Tclose(type);
   H5Dclose(vDataId);
   H5Gclose(vTimePointId);
   H5Gclose(vChannelId);
}