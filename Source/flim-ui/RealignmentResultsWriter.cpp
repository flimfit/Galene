#include "RealignmentResultsWriter.h"
#include <QFileInfo>
#include <QMessageBox>
#include "WriteMultipageTiff.h"
#include "Cv3dUtils.h"

void RealignmentResultsWriter::exportAlignedMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename)
{
   exportMovie(results, filename, &RealignmentResult::realigned);
}

void RealignmentResultsWriter::exportAlignedIntensityPreservingMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename)
{
   exportMovie(results, filename, &RealignmentResult::realigned_preserving);}
 
void RealignmentResultsWriter::exportUnalignedMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename)
{
   exportMovie(results, filename, &RealignmentResult::frame);
}

void RealignmentResultsWriter::exportCoverageMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename)
{
   exportMovie(results, filename, &RealignmentResult::mask);
}


void RealignmentResultsWriter::exportMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename, CachedObject<cv::Mat> RealignmentResult::*field)
{
   std::vector<cv::Mat> images;
   for (auto& p : results)
   {
      auto& r = p.second;
      cv::Mat b;
      const cv::Mat& im = r.*field;
      for (int z = 0; z < im.size[0]; z++)
      {
         extractSlice(im, z).convertTo(b, CV_16U);
         images.push_back(b);
      }
   }

   writeMovie(filename, images);
}


void RealignmentResultsWriter::writeMovie(const QString& filename, const std::vector<cv::Mat>& images)
{
   QString extension = QFileInfo(filename).suffix();

   if (extension == "avi")
   {
#ifndef SUPPRESS_OPENCV_HIGHGUI
      auto sz = images[0].size();
      auto writer = cv::VideoWriter(filename.toStdString(), -1, 4, sz, false);

      double mn, mx;
      cv::minMaxIdx(images[0], &mn, &mx);
      double scale = 255 / (0.8*mx);

      cv::Mat buf(sz, CV_8U);
      for (const auto& im : images)
      {
         im.convertTo(buf, CV_8U, scale);
         writer.write(buf);
      }
#else
      QMessageBox::warning(nullptr, "Could not export", "Cannot export when compiled in debug mode");
#endif
   }
   else if (extension == "tif")
   {
      writeMultipageTiff(filename.toStdString(), images);
   }
}