#pragma once

#include "ui_LifetimeDisplayWidget.h"
#include "FLIMage.h"

#include <opencv2/contrib/contrib.hpp>
#include <opencv2/core/core.hpp>

class LifetimeDisplayWidget : public QWidget, public Ui::LifetimeDisplayWidget
{
public:
   
   LifetimeDisplayWidget(QWidget* parent = 0) :
      QWidget(parent)
   {
      setupUi(this);
      lifetime_image_widget->layout()->setContentsMargins(QMargins(0, 0, 0, 0));

      setupPlots();
   }

   void setFLIMage(FLIMage** flimage_)
   {
      flimage = flimage_;
      connect(*flimage, &FLIMage::decayUpdated, this, &LifetimeDisplayWidget::updateDecay, Qt::QueuedConnection);
   }

protected:

   void updateDecay()
   {
      if (flimage == nullptr)
         return;

      auto& d = (*flimage)->getCurrentDecay();

      QVector<double> decay(d.size());
      QVector<double> t(d.size());
      for (int i = 0; i < decay.size(); i++)
      {
         decay[i] = d[i];
         t[i] = i;
      }

      decay_plot->graph(0)->setData(t, decay);
      decay_plot->rescaleAxes();
      decay_plot->replot();
   }

   void updateLifetimeImage()
   {
      if (flimage == nullptr)
         return;

      auto f = *flimage;

      cv::Mat& intensity = f->getIntensity();
      cv::Mat& mar = f->getMeanArrivalTime();

      double min_mar = 500;
      double max_mar = 5000;

      mar.convertTo(scaled_mar, CV_8U, 255.0 / (max_mar - min_mar), -255.0/min_mar);
      cv::applyColorMap(mar, display, cv::COLORMAP_JET);

      lifetime_image_widget->GetRenderWidget()->

   }

   void setupPlots()
   {
      decay_plot->addGraph();

      decay_plot->xAxis->setLabel("Time (ns)");
      decay_plot->yAxis->setLabel("Counts");
      decay_plot->replot();



      lifetime_histogram_plot->addGraph();

      lifetime_histogram_plot->xAxis->setLabel("Lifetime (ns)");
      lifetime_histogram_plot->yAxis->setLabel("Frequency");
      lifetime_histogram_plot->replot();

   }

   FLIMage** flimage = nullptr;
   cv::Mat display;
   cv::Mat scaled_mar;

};