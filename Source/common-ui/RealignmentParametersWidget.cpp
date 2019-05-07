#include "RealignmentParametersWidget.h"
#include <QMessageBox>

RealignmentParametersWidget::RealignmentParametersWidget(QWidget* parent) : 
   QWidget(parent), ControlBinder(parent, "RealignmentParametersWidget")
{
   setupUi(this);

   connect(mode_combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RealignmentParametersWidget::updateParameterGroupBox);
   connect(realign_button, &QPushButton::pressed, [&]() { emit realignRequested(getParameters()); });
   connect(reload_button, &QPushButton::pressed, [&]() { emit reloadRequested(getParameters()); });

   BindProperty(default_reference_combo, this, default_reference);
   BindProperty(use_gpu_check, this, use_gpu);
   BindProperty(mode_combo, this, mode);
   BindProperty(realignment_points_spin, this, realignment_points);
   BindProperty(smoothing_spin, this, smoothing);
   BindProperty(threshold_spin, this, threshold);
   BindProperty(coverage_threshold_spin, this, coverage_threshold);
   BindProperty(store_frames_check, this, store_frames);
   BindProperty(spatial_binning_combo, this, spatial_binning);
}

RealignmentParameters RealignmentParametersWidget::getParameters()
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

void RealignmentParametersWidget::setGpuSupportInfo(const GpuSupportInformation& support_info)
{
   use_gpu_check->setEnabled(support_info.supported);
   gpu_stacked_widget->setCurrentIndex(support_info.supported);
   gpu_info_label->setText(QString::fromStdString(support_info.message));

   if (support_info.remedy_message != "")
      QMessageBox::information(this, "GPU Support", QString::fromStdString(support_info.remedy_message));
}

void RealignmentParametersWidget::updateParameterGroupBox(int index)
{
   int page = index;
   parameter_stackedwidget->setCurrentIndex(page);
}