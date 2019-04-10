#pragma once

#include "RealignmentStudio.h"
#include "ThreadedObject.h"
#include "TaskProgress.h"
#include <QStringList>

class RealignmentStudioBatchProcessor : public QObject //: public ThreadedObject
{
   Q_OBJECT

public:
   RealignmentStudioBatchProcessor(RealignmentStudio* studio_, QStringList files_, bool align_together = false);

   void processNext();
   void saveCurrent();

private:

   bool align_together;
   cv::Mat reference_frame;

   RealignableDataOptions options; // start uninitialised
   std::shared_ptr<RealignableDataSource> source;

   std::shared_ptr<TaskProgress> task;
   RealignmentStudio* studio;
   QStringList files;
};
