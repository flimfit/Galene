#pragma once

#include "AbstractFrameAligner.h"
#include <QString>

class RealignmentResultsWriter
{
public:

   static void exportAlignedMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename);
   static void exportAlignedIntensityPreservingMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename);
   static void exportUnalignedMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename);
   static void exportCoverageMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename);
   static void exportMovie(const std::map<size_t,RealignmentResult>& results, const QString& filename, CachedObject<cv::Mat> RealignmentResult::*field);

protected:

   static void writeMovie(const QString& filename, const std::vector<cv::Mat>& images);
};