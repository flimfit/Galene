#pragma once

#include "GpuFrameWarper.h"
#include <QWidget>
#include "AbstractFrameAligner.h"
#include "ui_RealignmentParametersWidget.h"
#include "ControlBinder.h"

class RealignmentParametersWidget : public QWidget, public ControlBinder, public Ui::RealignmentParametersWidget
{
   Q_OBJECT

public:

   RealignmentParametersWidget(QWidget* parent);

   RealignmentParameters getParameters();
   void setGpuSupportInfo(const GpuSupportInformation& support_info);

signals:

   void realignRequested(const RealignmentParameters& params);
   void reloadRequested(const RealignmentParameters& params);

private:

   void updateParameterGroupBox(int index);

   bool use_gpu = true;
   int default_reference = 0;
   int mode = (int)RealignmentType::Warp;
   int realignment_points = 10;
   double smoothing = 2;
   double threshold = 0;
   double coverage_threshold = 0;
   bool store_frames = false;
   int spatial_binning = 0;

   Q_PROPERTY(bool use_gpu MEMBER use_gpu);
   Q_PROPERTY(int default_reference MEMBER default_reference);
   Q_PROPERTY(int mode MEMBER mode);
   Q_PROPERTY(int realignment_points MEMBER realignment_points);
   Q_PROPERTY(double smoothing MEMBER smoothing);
   Q_PROPERTY(double threshold MEMBER threshold);
   Q_PROPERTY(double coverage_threshold MEMBER coverage_threshold);
   Q_PROPERTY(bool store_frames MEMBER store_frames);
   Q_PROPERTY(int spatial_binning MEMBER spatial_binning);
};
