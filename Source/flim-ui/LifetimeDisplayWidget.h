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
      setupPlots();
      connect(lifetime_image_widget, &ImageRenderWidget::ConstrainWidth, this, &LifetimeDisplayWidget::setMaximumWidth);
   }

   void setFLIMage(FLIMage** flimage_)
   {
      flimage = flimage_;
      connect(*flimage, &FLIMage::decayUpdated, this, &LifetimeDisplayWidget::updateDecay, Qt::QueuedConnection);
      connect(*flimage, &FLIMage::decayUpdated, this, &LifetimeDisplayWidget::updateLifetimeImage, Qt::QueuedConnection);
   }

protected:

   void updateDecay()
   {
      if (flimage == nullptr)
         return;

      auto& d = (*flimage)->getCurrentDecay();

      int n = static_cast<int>(d.size());
      QVector<double> decay(n);
      QVector<double> t(n);

      int n_tot = 0;

      for (int i = 0; i < n; i++)
      {
         n_tot += d[i];
         decay[i] = d[i];
         t[i] = i;
      }

      //std::cout << n_tot << "\n";

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

      double min_mar = 30;
      double max_mar = 200;

      mar.convertTo(scaled_mar, CV_8U, 255.0 / (max_mar - min_mar), -255.0 / (max_mar - min_mar));
      cv::applyColorMap(scaled_mar, scaled_mar, cv::COLORMAP_JET);

      double i_min, i_max;
      cv::minMaxLoc(intensity, &i_min, &i_max);
      i_min = 0;
      intensity.convertTo(alpha, CV_8U, 255.0 / (i_max - i_min), -255.0 / (i_max - i_min));
      

      display = cv::Mat(scaled_mar.rows, scaled_mar.cols, CV_8UC4);
      cv::mixChannels({ { scaled_mar, alpha } }, { { display } }, { 0, 0, 1, 1, 2, 2, 3, 3 }); // to ARGB1

      lifetime_image_widget->SetImage(display);
   }

   void setupPlots()
   {
      decay_plot->addGraph();
      decay_plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));

      decay_plot->xAxis->setLabel("Time (ns)");
      decay_plot->yAxis->setLabel("Counts");

      decay_plot->replot();



      lifetime_histogram_plot->addGraph();

      lifetime_histogram_plot->xAxis->setLabel("Lifetime (ns)");
      lifetime_histogram_plot->yAxis->setLabel("Frequency");
      lifetime_histogram_plot->replot();

   }

   FLIMage** flimage = nullptr;
   cv::Mat mapped_mar;
   cv::Mat scaled_mar;
   cv::Mat display;
   cv::Mat alpha;

};