#pragma once

#include "ui_RealignmentParametersWidget.h"
#include "ControlBinder.h"
#include "GpuFrameWarper.h"
#include <QWidget>

class RealignmentParametersWidget : public QWidget, public ControlBinder, public Ui::RealignmentParametersWidget
{
   Q_OBJECT

public:

   RealignmentParametersWidget(QWidget* parent) : 
      QWidget(parent), ControlBinder(parent, "RealignmentParametersWidget")
   {
      setupUi(this);

      BindProperty(default_reference_combo, this, default_reference);
      BindProperty(use_gpu_check, this, use_gpu);
      BindProperty(mode_combo, this, mode);
      BindProperty(realignment_points_spin, this, realignment_points);
      BindProperty(smoothing_spin, this, smoothing);
      BindProperty(threshold_spin, this, threshold);
      BindProperty(coverage_threshold_spin, this, coverage_threshold);
      BindProperty(store_frames_check, this, store_frames);
      BindProperty(spatial_binning_combo, this, spatial_binning);

      connect(mode_combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RealignmentParametersWidget::updateParameterGroupBox);

      connect(realign_button, &QPushButton::pressed, [&]() { emit realignRequested(getParameters()); });
      connect(reload_button, &QPushButton::pressed, [&]() { emit reloadRequested(getParameters()); });
   }

   RealignmentParameters getParameters()
   {
      RealignmentParameters params;

      params.type = static_cast<RealignmentType>(mode_combo->currentIndex());
      params.n_resampling_points = realignment_points_spin->value();
      
      switch (params.type)
      {
      case RealignmentType::Warp:
         params.frame_binning = 1;
         params.spatial_binning = 4;
         break;
      default:
         params.frame_binning = frame_binning_combo->value();
         params.spatial_binning = pow(2, spatial_binning_combo->currentIndex());
      }

      // Fix spatial binning until we expose GUI properly
      params.spatial_binning = pow(2, spatial_binning_combo->currentIndex());

      params.correlation_threshold = threshold_spin->value();
      params.coverage_threshold = coverage_threshold_spin->value() / 100.;
      params.smoothing = smoothing_spin->value();
      params.prefer_gpu = use_gpu_check->isChecked();
      params.default_reference_frame = (DefaultReferenceFrame) default_reference_combo->currentIndex();
      params.store_frames = store_frames_check->isChecked();

      return params;
   }

   void setGpuSupportInfo(const GpuSupportInformation& support_info)
   {
      use_gpu_check->setEnabled(support_info.supported);
      gpu_stacked_widget->setCurrentIndex(support_info.supported);
      gpu_info_label->setText(QString::fromStdString(support_info.message));

      if (support_info.remedy_message != "")
         QMessageBox::information(this, "GPU Support", QString::fromStdString(support_info.remedy_message));
   }

signals:

   void realignRequested(const RealignmentParameters& params);
   void reloadRequested(const RealignmentParameters& params);

private:

   void updateParameterGroupBox(int index)
   {
      int page = index;
      parameter_stackedwidget->setCurrentIndex(page);
   }

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