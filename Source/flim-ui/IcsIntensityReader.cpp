#include "IcsIntensityReader.h"
#include <Cv3dUtils.h>
#include <mutex>

std::mutex ics_mutex;


void check(Ics_Error err)
{
   if (err != IcsErr_Ok)
      throw std::runtime_error(IcsGetErrorText(err));
}

IcsIntensityReader::IcsIntensityReader(const std::string& filename) :
   IntensityReader(filename)
{
   check(IcsOpen(&ics, filename.c_str(), "r"));
   readMetadata();
}

IcsIntensityReader::~IcsIntensityReader()
{
   if (ics)
      IcsClose(ics);
   ics = nullptr;
}

QStringList IcsIntensityReader::supportedExtensions()
{
   return { "ics" };
}

void IcsIntensityReader::readMetadata()
{
   std::lock_guard<std::mutex> lk(ics_mutex);

   Ics_DataType data_type;
   int ndims;
   size_t dims[ICS_MAXDIM];

   IcsGetLayout (ics, &data_type, &ndims, dims);

   switch (data_type)
   {
   case Ics_uint8:
      cv_type = CV_8U; break;
   case Ics_sint8:
      cv_type = CV_8S; break;
   case Ics_uint16:
      cv_type = CV_16U; break;
   case Ics_sint16:
      cv_type = CV_16S; break;
   case Ics_uint32:
   case Ics_sint32:
      cv_type = CV_32S; break;
   case Ics_real32:
      cv_type = CV_32F; break;
   case Ics_real64:
      cv_type = CV_64F; break;
   default:
      throw std::runtime_error("unsigned unsupported ICS data type");

   }

   const char* order;
   const char* label;
   for(int d=0; d<ndims; d++)
   {
      check(IcsGetOrderF(ics, d, &order, &label));
      if (strcmp(order, "x") == 0)
      {
         dim_order.x = d;
         n_x = dims[d];
      }
      else if (strcmp(order, "y") == 0)
      {
         dim_order.y = d;
         n_y = dims[d];
      }
      else if (strcmp(order, "z") == 0)
      {
         dim_order.z = d;
         n_z = dims[d];
      }
      else if (strcmp(order, "t") == 0)
      {
         dim_order.t = d;
         n_t = dims[d];
      }
      else
      {
         dim_order.c = d;
         n_chan = dims[d];
      }
   }

   bool bidirectional = false;
   scan_params = ImageScanParameters(1000, 1000, 1000 * n_y, n_x, n_y, n_z, bidirectional);

   setUseAllChannels();
}

void IcsIntensityReader::addStack(int chan, int t, cv::Mat& data)
{
   std::lock_guard<std::mutex> lk(ics_mutex);

   size_t size[5];
   size[dim_order.x] = n_x;
   size[dim_order.y] = n_y;
   size[dim_order.z] = n_z;
   size[dim_order.c] = 1;
   size[dim_order.t] = 1;

   size_t offset[5];
   offset[dim_order.x] = 0;
   offset[dim_order.y] = 0;
   offset[dim_order.z] = 0;
   offset[dim_order.c] = chan;
   offset[dim_order.t] = t;

   std::vector<int> cv_dims = {n_z, n_y, n_x};
   cv::Mat buf(cv_dims, cv_type);

   size_t buf_size = buf.elemSize1() * n_z * n_y * n_x;
   check(IcsGetROIData(ics, offset, size, NULL, buf.data, buf_size));
   
   cv::Mat cv16;
   buf.convertTo(cv16, CV_16U);
   data += cv16;
}
