#pragma once

#include "ImageRenderWindow.h"
#include "FlimReaderDataSource.h"
#include <memory>

class RealignmentImageSource : public QObject
{
   Q_OBJECT
public:

   RealignmentImageSource(std::shared_ptr<FlimReaderDataSource> reader, ImageRenderWindow* window, QObject* parent = 0) :
      QObject(parent), reader(reader), window(window)
   {
      connect(reader.get(), &FlimReaderDataSource::alignmentComplete, this, &RealignmentImageSource::update);
   }

protected:

   void update()
   {
      ImageRenderWidget* w = window->GetRenderWidget();
      w->ClearImages();

      auto r = reader->getReader();
      auto results = r->getRealignmentResults();

      QVector<double> corr;
      corr.reserve(results.size());
      for (auto& r : results)
      {
         cv::Mat H;
         cv::hconcat(r.frame, r.realigned, H);
         w->AddImage(H);
         corr.push_back(r.correlation);
      }
   }

   ImageRenderWindow* window;
   std::shared_ptr<FlimReaderDataSource> reader;
   cv::Mat current_frame;
};