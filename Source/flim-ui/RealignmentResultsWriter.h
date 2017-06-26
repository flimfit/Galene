#pragma once

#include "AbstractFrameAligner.h"
#include <QString>

class RealignmentResultsWriter
{
public:

   static void exportAlignedMovie(const std::vector<RealignmentResult>& results, const QString& filename);
   static void exportAlignedIntensityPreservingMovie(const std::vector<RealignmentResult>& results, const QString& filename);
   static void exportUnalignedMovie(const std::vector<RealignmentResult>& results, const QString& filename);
   static void exportCoverageMovie(const std::vector<RealignmentResult>& results, const QString& filename);

protected:

   static void writeMovie(const QString& filename, const std::vector<cv::Mat>& images);
};