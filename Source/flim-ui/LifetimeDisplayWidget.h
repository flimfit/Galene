#pragma once

#include "ui_LifetimeDisplayWidget.h"
#include "ControlBinder.h"
#include "FlimDataSource.h"
#include <memory>

#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>

class LifetimeDisplayWidget : public QWidget, public Ui::LifetimeDisplayWidget, public ControlBinder
{
   Q_OBJECT

public:
   
   LifetimeDisplayWidget(QWidget* parent = 0);
   void setFlimDataSource(std::shared_ptr<FlimDataSource> flimage_);

   void updateCountRates();
   void setClosable(bool closable_) { closable = closable_; }

   void closeEvent(QCloseEvent *event);

signals:

   void displayTauMinChanged(double);
   void displayTauMaxChanged(double);
   void displayIntensityMinChanged(int);
   void displayIntensityMaxChanged(int);

protected:

   void setAutoscaleIntensity(bool autoscale_intensity_);
   void setAutoscaleTau(bool autoscale_tau_);
   void setDisplayTauMin(double display_tau_min_);
   void setDisplayTauMax(double display_tau_max_);
   void setDisplayIntensityMin(int display_intensity_min_);
   void setDisplayIntensityMax(int display_intensity_max_);
   
   bool getAutoscaleTau() { return autoscale_tau; }
   bool getAutoscaleIntensity() { return autoscale_intensity; }
   double getDisplayTauMin() { return display_tau_min; };
   double getDisplayTauMax() { return display_tau_max; }
   int getDisplayIntensityMin() { return display_intensity_min; };
   int getDisplayIntensityMax() { return display_intensity_max; };


   void updateDecay();

   void updateLifetimeImage() { updateLifetimeImageImpl(false); }
   void updateLifetimeScale() { updateLifetimeImageImpl(false); }

   void updateLifetimeImageImpl(bool rescale);
   void setupPlots();

   std::shared_ptr<FlimDataSource> flimage;
   cv::Mat mapped_mar;
   cv::Mat scaled_mar;
   cv::Mat display;
   cv::Mat alpha;

   std::vector<QColor> line_colors;
   std::vector<QLabel*> rate_labels;

   double display_tau_min = 1;
   double display_tau_max = 5;
   bool autoscale_tau = false;
   int display_intensity_min = 0;
   int display_intensity_max = 100;
   bool autoscale_intensity = true;
   bool closable = true;

   QString unit = "ps";
   double scale_factor = 1e-3;
};