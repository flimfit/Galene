#pragma once

#include "ui_LifetimeDisplayWidget.h"
#include "FLIMage.h"
#include <memory>

#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>

class LifetimeDisplayWidget : public QWidget, public Ui::LifetimeDisplayWidget, public ControlBinder
{
   Q_OBJECT

public:
   
   LifetimeDisplayWidget(QWidget* parent = 0) :
      QWidget(parent),
      ControlBinder(parent, "LifetimeDisplayWidget")
   {
      setupUi(this);
      setupPlots();
      connect(lifetime_image_widget, &ImageRenderWidget::ConstrainWidth, this, &LifetimeDisplayWidget::setMaximumWidth);

      line_colors.push_back(Qt::blue);
      line_colors.push_back(Qt::red);
      line_colors.push_back(Qt::green);
      line_colors.push_back(Qt::magenta);

      QueuedBind(tau_auto_check, this, &LifetimeDisplayWidget::setAutoscaleTau, &LifetimeDisplayWidget::getAutoscaleTau);
      QueuedBind(intensity_auto_check, this, &LifetimeDisplayWidget::setAutoscaleIntensity, &LifetimeDisplayWidget::getAutoscaleIntensity);
      QueuedBind(tau_min_spin, this, &LifetimeDisplayWidget::setDisplayTauMin, &LifetimeDisplayWidget::getDisplayTauMin, &LifetimeDisplayWidget::displayTauMinChanged);
      QueuedBind(tau_max_spin, this, &LifetimeDisplayWidget::setDisplayTauMax, &LifetimeDisplayWidget::getDisplayTauMax, &LifetimeDisplayWidget::displayTauMaxChanged);
      QueuedBind(intensity_min_spin, this, &LifetimeDisplayWidget::setDisplayIntensityMin, &LifetimeDisplayWidget::getDisplayIntensityMin, &LifetimeDisplayWidget::displayIntensityMinChanged);
      QueuedBind(intensity_max_spin, this, &LifetimeDisplayWidget::setDisplayIntensityMax, &LifetimeDisplayWidget::getDisplayIntensityMax, &LifetimeDisplayWidget::displayIntensityMaxChanged);

      connect(rate_type_combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LifetimeDisplayWidget::updateCountRates);
   }

   void setFLIMage(std::shared_ptr<FLIMage> flimage_)
   {
      flimage = flimage_;
      connect(flimage.get(), &FLIMage::decayUpdated, this, &LifetimeDisplayWidget::updateDecay, Qt::QueuedConnection);
      connect(flimage.get(), &FLIMage::decayUpdated, this, &LifetimeDisplayWidget::updateLifetimeImage, Qt::QueuedConnection);
      connect(flimage.get(), &FLIMage::countRatesUpdated, this, &LifetimeDisplayWidget::updateCountRates, Qt::QueuedConnection);

      while (auto w = count_rate_layout->findChild<QWidget*>())
         delete w;

      int n_chan = flimage->getNumChannels();
      for (int i = 0; i < n_chan; i++)
      {
         QPalette p;
         p.setColor(QPalette::Window, line_colors[i].lighter());

         auto w = new QLabel("---");
         w->setFrameShape(QFrame::Shape::StyledPanel);
         w->setMargin(2);
         w->setAlignment(Qt::AlignCenter);
         w->setAutoFillBackground(true);
         w->setPalette(p);
         count_rate_layout->addWidget(w);

         rate_labels.push_back(w);
      }
   }

   void updateCountRates()
   {      
      if (!flimage)
         return;

      auto& rates = (rate_type_combo->currentIndex()) ? flimage->getMaxInstantCountRates() : flimage->getCountRates();
      QStringList label = { "", "k", "M", "G" };

      for (int i = 0; i < rate_labels.size(); i++)
      {
         int idx = 0;
         double rate = rates[i];
         int dp = 0;
         while ((rate > 1e3) && (idx < label.size()-1))
         {
            rate *= 1e-3;
            idx++;
            dp = 1;
         }
         QString text = QString("%1 %2cps").arg(rate,0,'f',dp).arg(label[idx]);
         rate_labels[i]->setText(text);
      }
   }

signals:

   void displayTauMinChanged(double);
   void displayTauMaxChanged(double);
   void displayIntensityMinChanged(int);
   void displayIntensityMaxChanged(int);

protected:

   void setAutoscaleIntensity(bool autoscale_intensity_)
   {
      autoscale_intensity = autoscale_intensity_;
      updateLifetimeScale();
   }

   void setAutoscaleTau(bool autoscale_tau_)
   {
      autoscale_tau = autoscale_tau_;
      updateLifetimeScale();
   }

   void setDisplayTauMin(double display_tau_min_)
   {
      display_tau_min = display_tau_min_;
      updateLifetimeScale();
   }
   
   void setDisplayTauMax(double display_tau_max_)
   {
      display_tau_max = display_tau_max_;
      updateLifetimeScale();
   }

   void setDisplayIntensityMin(int display_intensity_min_)
   {
      display_intensity_min = display_intensity_min_;
      updateLifetimeScale();
   }

   void setDisplayIntensityMax(int display_intensity_max_)
   {
      display_intensity_max = display_intensity_max_;
      updateLifetimeScale();
   }
   
   bool getAutoscaleTau() { return autoscale_tau; }
   bool getAutoscaleIntensity() { return autoscale_intensity; }
   double getDisplayTauMin() { return display_tau_min; };
   double getDisplayTauMax() { return display_tau_max; }
   int getDisplayIntensityMin() { return display_intensity_min; };
   int getDisplayIntensityMax() { return display_intensity_max; };


   void updateDecay()
   {
      if (flimage == nullptr)
         return;

      int n_chan = flimage->getNumChannels();
      int n_graphs = decay_plot->graphCount();

      for (int i = n_graphs; i < n_chan; i++)
      {
         int color_idx = i % line_colors.size();

         decay_plot->addGraph();
         decay_plot->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
         decay_plot->graph(i)->setPen(line_colors[color_idx]);
      }

      for (int c = 0; c < n_chan; c++)
      {

         auto& d = flimage->getCurrentDecay(c);
         double time_resolution = flimage->getTimeResolution();

         int n = static_cast<int>(d.size());
         QVector<double> decay(n);
         QVector<double> t(n);

         int n_tot = 0;

         for (int i = 0; i < n; i++)
         {
            n_tot += d[i];
            decay[i] = d[i];
            t[i] = i * time_resolution * 1e-3;
         }

         decay_plot->graph(c)->setData(t, decay);
         decay_plot->rescaleAxes();
         decay_plot->replot();
      }
   }

   void updateLifetimeImage()
   {
      updateLifetimeImageImpl(true);
   }

   void updateLifetimeScale()
   {
      updateLifetimeImageImpl(false);
   }

   void updateLifetimeImageImpl(bool rescale)
   {
      if (flimage == nullptr)
         return;

      cv::Mat intensity = flimage->getIntensity();
      cv::Mat mar = flimage->getMeanArrivalTime();

      double mar_min, mar_max;
      if (autoscale_tau && rescale)
      {
         cv::minMaxLoc(mar, &mar_min, &mar_max);

         display_tau_min = mar_min * 1e-3;
         display_tau_max = mar_max * 1e-3;

         emit displayTauMinChanged(display_tau_min);
         emit displayTauMaxChanged(display_tau_max);
      }
      else
      {
         mar_min = display_tau_min * 1e3;
         mar_max = display_tau_max * 1e3;
      }

      mar.convertTo(scaled_mar, CV_8U, 255.0 / (mar_max - mar_min), -255.0 * mar_min / (mar_max - mar_min));

      cv::minMaxLoc(scaled_mar, &mar_min, &mar_max);


      cv::applyColorMap(scaled_mar, mapped_mar, cv::COLORMAP_JET);

      double i_min, i_max;
      if (autoscale_intensity && rescale)
      {
         cv::minMaxLoc(intensity, &i_min, &i_max);
         display_intensity_min = 0;
         display_intensity_max = i_max;
         emit displayIntensityMinChanged(display_intensity_min);
         emit displayIntensityMaxChanged(display_intensity_max);
      }
      else
      {
         i_min = display_intensity_min;
         i_max = display_intensity_max;
      }

      intensity.convertTo(alpha, CV_8U, 255.0 / (i_max - i_min), -255.0 * i_min / (i_max - i_min));     

      display = cv::Mat(mapped_mar.rows, mapped_mar.cols, CV_8UC4);
      cv::mixChannels({ { mapped_mar, alpha } }, { { display } }, { 0, 0, 1, 1, 2, 2, 3, 3 }); // to ARGB1

      lifetime_image_widget->SetImage(display);
   }

   void setupPlots()
   {
      decay_plot->xAxis->setLabel("Time (ns)");
      decay_plot->yAxis->setLabel("Counts");

      decay_plot->replot();



      lifetime_histogram_plot->addGraph();

      lifetime_histogram_plot->xAxis->setLabel("Lifetime (ns)");
      lifetime_histogram_plot->yAxis->setLabel("Frequency");
      lifetime_histogram_plot->replot();

   }

   std::shared_ptr<FLIMage> flimage;
   cv::Mat mapped_mar;
   cv::Mat scaled_mar;
   cv::Mat display;
   cv::Mat alpha;

   std::vector<QColor> line_colors;
   std::vector<QLabel*> rate_labels;

   double display_tau_min;
   double display_tau_max;
   bool autoscale_tau = true;
   int display_intensity_min;
   int display_intensity_max;
   bool autoscale_intensity = true;
};