#include "RealignmentStudioBatchProcessor.h"


RealignmentStudioBatchProcessor::RealignmentStudioBatchProcessor(RealignmentStudio* studio, QStringList files, bool align_together) :
   studio(studio), files(files), align_together(align_together)
{
   task = std::make_shared<TaskProgress>("Processing...", true, files.size());
   TaskRegister::addTask(task);

   processNext();
}

void RealignmentStudioBatchProcessor::processNext()
{
   if (files.isEmpty() || task->wasCancelRequested())
   {
      source = nullptr;
      task->setFinished();
      return;
   }

   auto file = files.first();
   files.removeFirst();

   source = studio->openFileWithOptions(file, options);
   if (source)
   {
      source->setReferenceFrame(reference_frame);
      connect(source.get(), &RealignableDataSource::readComplete, this, &RealignmentStudioBatchProcessor::saveCurrent);
   }
   else
   {
      task->incrementStep();
      processNext();
   }
}

void RealignmentStudioBatchProcessor::saveCurrent()
{
   disconnect(source.get(), &RealignableDataSource::readComplete, this, &RealignmentStudioBatchProcessor::saveCurrent);

   if (align_together && reference_frame.empty())
      reference_frame = source->getReferenceFrame();

   studio->save(source, true);
   task->incrementStep();
   processNext();
}