#include "IcsIntensityReader.h"
#include <Cv3dUtils.h>
#include <mutex>

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
   Ics_DataType dt;
   int ndims;
   size_t dims[ICS_MAXDIM];

   IcsGetLayout (ics, &dt, &ndims, dims);

   const char* order;
   const char* label;
   for(int d=0; d<ndims; d++)
   {
      check(IcsGetOrderF(ics, d, &order, &label));
      if (strcmp(order,"x")==0)
         n_x = dims[d];
      else if (strcmp(order,"y")==0)
         n_y = dims[d];
      else if (strcmp(order,"z")==0)
         n_z = dims[d];
      else if (strcmp(order,"t")==0)
         n_t = dims[d];
      else
         n_chan = dims[d];
   }

   bool bidirectional = false;
   scan_params = ImageScanParameters(1000, 1000, 1000 * n_y, n_x, n_y, n_z, bidirectional);

   setUseAllChannels();
}

void IcsIntensityReader::addStack(int chan, int t, cv::Mat& data)
{
   size_t size[5];
   size[x_dim] = n_x;
   size[y_dim] = n_y;
   size[z_dim] = n_z;
   size[t_dim] = n_t;
   size[c_dim] = n_chan;

   size_t offset[5];


   /*
   bufsize = IcsGetDataSize (ics);
   buf = malloc (bufsize);
   if (buf == NULL)
   check(IcsGetData(ics, buf, bufsize));
   */

   std::vector<int> cv_dims = {n_z, n_y, n_x};
   cv::Mat buf(cv_dims, cv_type);
   check(IcsGetROIData(ics, offset, size, 1, buf.data, size_t n));
   
   cv::Mat cv16;
   buf.convertTo(cv16, CV_16U);
   data += cv16;
}
