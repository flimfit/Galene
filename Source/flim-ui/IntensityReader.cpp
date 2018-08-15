#include "IntensityReader.h"
#include <QFileInfo>

#include "OmeIntensityWriter.h"
#include "OmeIntensityReader.h"
#include "BioImageIntensityReader.h"
#include "ImarisIntensityReader.h"
#include "IcsIntensityReader.h"

#include "Cache_impl.h"

IntensityReader::IntensityReader(const std::string& filename) : 
   AligningReader(), filename(filename)
{
      async_load_intensity_frames = true;
}

QStringList IntensityReader::supportedExtensions()
{
   QStringList exts;
   exts.append(OmeIntensityReader::supportedExtensions());
   exts.append(BioImageIntensityReader::supportedExtensions());
   exts.append(ImarisIntensityReader::supportedExtensions());
   exts.append(IcsIntensityReader::supportedExtensions());
   return exts;
}


std::shared_ptr<IntensityReader> IntensityReader::getReader(const std::string& filename)
{
   QFileInfo file(QString::fromStdString(filename));
   QString ext = file.completeSuffix();

   if (OmeIntensityReader::supportedExtensions().contains(ext))
      return std::make_shared<OmeIntensityReader>(filename);
   
   if (BioImageIntensityReader::supportedExtensions().contains(ext))
      return std::make_shared<BioImageIntensityReader>(filename);

   if (ImarisIntensityReader::supportedExtensions().contains(ext))
      return std::make_shared<ImarisIntensityReader>(filename);

   if (IcsIntensityReader::supportedExtensions().contains(ext))
      return std::make_shared<IcsIntensityReader>(filename);

   throw std::runtime_error("Unsupported file type");
}

void IntensityReader::read()
{
   waitForAlignmentComplete();
}

double IntensityReader::getProgress()
{
   return 0; //TODO
   /*
   if (realignment.empty()) return 0;

   int n_done = 0;
   for (int i = 0; i < realignment.size(); i++)
      n_done += realignment[i].done;

   return static_cast<double>(n_done) / frames.size();
   */
}


cv::Mat IntensityReader::getStack(int chan, int t)
{
   std::vector<int> dims = { n_z, n_y, n_x };
   cv::Mat stack(dims, CV_16U, cv::Scalar(0));
   addStack(chan, t, stack);
   return stack;
}


void IntensityReader::loadIntensityFramesImpl()
{
   auto cache = Cache<cv::Mat>::getInstance();
   for (int t = 0; t < n_t; t++)
   {
      auto fcn = std::bind(&IntensityReader::getIntensityFrameImmediately, this, t);
      frames[t] = cache->add(fcn);
      frames[t].get(); // preload into cache
   }
}

cv::Mat IntensityReader::getIntensityFrameImmediately(int t)
{
   std::vector<int> dims = { n_z, n_y, n_x };
   cv::Mat frame(dims, CV_16U, cv::Scalar(0));
   for (int chan = 0; chan < n_chan; chan++)
      if (use_channel[chan])
         addStack(chan, t, frame);
   return frame;
}


cv::Mat IntensityReader::getRealignedStack(int chan, int t, bool interpolate)
{
   cv::Mat realigned;
   cv::Mat stack = getStack(chan, t);
   stack.convertTo(stack, CV_32F);
   
   if (frame_aligner)
      realigned = frame_aligner->realignAsFrame(t, stack, interpolate);
   else
      realigned = stack;

   return realigned;
}
