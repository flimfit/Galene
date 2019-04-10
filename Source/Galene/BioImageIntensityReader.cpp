#include "BioImageIntensityReader.h"
#include <Cv3dUtils.h>

BioImageIntensityReader::BioImageIntensityReader(const std::string& filename) :
      IntensityReader(filename)
{
   // Create TIFF reader
   image5d = std::make_shared<bim::Image5D>();
   image5d->fromFile(filename);
   readMetadata();
}

QStringList BioImageIntensityReader::supportedExtensions()
{
   return { "lsm", "tif" };
}


void BioImageIntensityReader::readMetadata()
{
   bool bidirectional = false;

   n_x = image5d->numberX();
   n_y = image5d->numberY();
   n_z = image5d->numberZ();
   n_t = image5d->numberT();
   n_chan = image5d->numberC();

   swap_zt = (n_t == 1 && n_z > 1);
   if (swap_zt)
   {
      int nb = n_t;
      n_t = n_z;
      n_z = nb;
      std::cout << "Swapping z and t\n";
   }

   // Get underlying cv type
   cv_type = image5d->imageAt(0, 0)->asCVMat(0).type();
   
   setUseAllChannels();   
   scan_params = ImageScanParameters(100, 101, 101 * (n_y + 1), n_x, n_y, n_z, bidirectional);
}

void BioImageIntensityReader::addStack(int chan, int t, cv::Mat& data)
{
   std::lock_guard<std::mutex> lk(read_mutex);
   
   cv::Mat cv16;

   for (int z = 0; z < n_z; z++)
   {
      try
      {
         auto im = swap_zt ? image5d->imageAt(z, t) : image5d->imageAt(t, z);
         cv::Mat cvbuf = im->asCVMat(chan);         
         cvbuf.convertTo(cv16, CV_16U);

         // Copy into frame
         if (cvbuf.size().width == n_x && cvbuf.size().height == n_y)
            extractSlice(data, z) += cv16;
         else  
            std::cout << "Slice (Z:" << z << ", C: " << chan << ", T: " << t << ") is the wrong size!" << "\n";
      }
      catch (std::exception e)
      {
         std::cout << "Error extracting slice: " << e.what() << "\n";
      }
   }
}