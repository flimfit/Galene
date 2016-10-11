#pragma once

#pragma once

#include "ui_RealignmentDisplayWidget.h"
#include "ControlBinder.h"
#include <memory>

#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>

class RealignmentDisplayWidget : public QWidget, public Ui::RealignmentDisplayWidget, public ControlBinder
{
   Q_OBJECT

public:

   RealignmentDisplayWidget(QWidget* parent = 0);

   void setClosable(bool closable_) { closable = closable_; }

   void closeEvent(QCloseEvent *event);

signals:

protected:

   void setupPlots();

   std::vector<QColor> line_colors;
   std::vector<QLabel*> rate_labels;

   double display_tau_min = 1;
   double display_tau_max = 5;
   bool autoscale_tau = false;
   int display_intensity_min = 0;
   int display_intensity_max = 100;
   bool autoscale_intensity = true;
   bool closable = true;
};