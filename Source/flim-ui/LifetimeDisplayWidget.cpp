#include "LifetimeDisplayWidget.h"
#include "Cv3dUtils.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>


LifetimeDisplayWidget::LifetimeDisplayWidget(QWidget* parent) :
  ControlBinder(this, "LifetimeDisplayWidget")
{
   setupUi(this);
   setupPlots();
   connect(lifetime_image_widget, &ImageRenderWidget::ConstrainWidth, this, &LifetimeDisplayWidget::setMaximumWidth);

   line_colors.push_back(Qt::blue);
   line_colors.push_back(Qt::green);
   line_colors.push_back(Qt::red);
   line_colors.push_back(Qt::magenta);

   Bind(tau_auto_check, this, &LifetimeDisplayWidget::setAutoscaleTau, &LifetimeDisplayWidget::getAutoscaleTau, &LifetimeDisplayWidget::autoscaleLifetimeChanged, true);
   Bind(intensity_auto_check, this, &LifetimeDisplayWidget::setAutoscaleIntensity, &LifetimeDisplayWidget::getAutoscaleIntensity, &LifetimeDisplayWidget::autoscaleIntensityChanged, true);
   QueuedBind(tau_min_spin, this, &LifetimeDisplayWidget::setDisplayTauMin, &LifetimeDisplayWidget::getDisplayTauMin, &LifetimeDisplayWidget::displayTauMinChanged);
   QueuedBind(tau_max_spin, this, &LifetimeDisplayWidget::setDisplayTauMax, &LifetimeDisplayWidget::getDisplayTauMax, &LifetimeDisplayWidget::displayTauMaxChanged);
   QueuedBind(intensity_min_spin, this, &LifetimeDisplayWidget::setDisplayIntensityMin, &LifetimeDisplayWidget::getDisplayIntensityMin, &LifetimeDisplayWidget::displayIntensityMinChanged);
   QueuedBind(intensity_max_spin, this, &LifetimeDisplayWidget::setDisplayIntensityMax, &LifetimeDisplayWidget::getDisplayIntensityMax, &LifetimeDisplayWidget::displayIntensityMaxChanged);

   connect(rate_type_combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LifetimeDisplayWidget::updateCountRates);

   connect(z_scroll, &QScrollBar::valueChanged, this, &LifetimeDisplayWidget::updateLifetimeImage);
}

void LifetimeDisplayWidget::setFlimDataSource(FlimDataSource* flimage_)
{
   //FlimDataSourceWatcher::setFlimDataSource(flimage_);

   flimage = flimage_;
   connect(flimage, &FlimDataSource::readComplete, this, &LifetimeDisplayWidget::updateLifetimeImage, Qt::QueuedConnection);
   connect(flimage, &FlimDataSource::decayUpdated, this, &LifetimeDisplayWidget::updateDecay, Qt::QueuedConnection);
   connect(flimage, &FlimDataSource::decayUpdated, this, &LifetimeDisplayWidget::updateLifetimeScale, Qt::QueuedConnection);
   connect(flimage, &FlimDataSource::countRatesUpdated, this, &LifetimeDisplayWidget::updateCountRates, Qt::QueuedConnection);

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

   updateLifetimeImage();
   updateDecay();
}

void LifetimeDisplayWidget::updateCountRates()
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
      while ((rate > 1e3) && (idx < label.size() - 1))
      {
         rate *= 1e-3;
         idx++;
         dp = 1;
      }
      QString text = QString("%1 %2cps").arg(rate, 0, 'f', dp).arg(label[idx]);
      rate_labels[i]->setText(text);
   }
}

void LifetimeDisplayWidget::saveMergedImage()
{
   QString file = QFileDialog::getSaveFileName(this, "Choose file name", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "Tiff (*.tif)");
   if (file.isEmpty()) return;

   // Intensity merge
   cv::Mat merged, alpha3;
   cv::Mat in[] = { alpha, alpha, alpha };
   cv::merge(in, 3, alpha3);
   cv::multiply(mapped_mar, alpha3, merged, 1.0/255);
#ifndef SUPPRESS_OPENCV_HIGHGUI
   cv::imwrite(file.toStdString(), merged);
#endif
}

void LifetimeDisplayWidget::closeEvent(QCloseEvent *event)
{
   if (closable)
      event->accept();
   else
      event->ignore();
}


void LifetimeDisplayWidget::setAutoscaleIntensity(bool autoscale_intensity_)
{
   autoscale_intensity = autoscale_intensity_;
   updateLifetimeScale();
}

void LifetimeDisplayWidget::setAutoscaleTau(bool autoscale_tau_)
{
   autoscale_tau = autoscale_tau_;
   updateLifetimeScale();
}

void LifetimeDisplayWidget::setDisplayTauMin(double display_tau_min_)
{
   display_tau_min = display_tau_min_;
   updateLifetimeScale();
}

void LifetimeDisplayWidget::setDisplayTauMax(double display_tau_max_)
{
   display_tau_max = display_tau_max_;
   updateLifetimeScale();
}

void LifetimeDisplayWidget::setDisplayIntensityMin(int display_intensity_min_)
{
   display_intensity_min = display_intensity_min_;
   updateLifetimeScale();
}

void LifetimeDisplayWidget::setDisplayIntensityMax(int display_intensity_max_)
{
   display_intensity_max = display_intensity_max_;
   updateLifetimeScale();
}

void LifetimeDisplayWidget::updateDecay()
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
         t[i] = i * time_resolution * scale_factor;
      }

      decay_plot->graph(c)->setData(t, decay);
      decay_plot->rescaleAxes();
      decay_plot->replot();
   }
}

void LifetimeDisplayWidget::updateLifetimeImageImpl(bool rescale)
{
   cv::Mat intensity, mar;

   if (flimage == nullptr)
      return;

   cv::Mat intensity_all = flimage->getIntensity();
   cv::Mat mar_all = flimage->getMeanArrivalTime();

   if (intensity_all.dims > 2) // 3D data
   {
      z_scroll->setMaximum(intensity_all.size[0] - 1);
      z = z_scroll->value();

      showZscroll(intensity_all.size[0] > 1);

      // Pull out required slice
      intensity = extractSlice(intensity_all, z);
      mar = extractSlice(mar_all, z);
   }
   else
   {
      showZscroll(false);
      intensity = intensity_all;
      mar = mar_all;
   }


   double mar_min, mar_max;
   if (autoscale_tau && rescale)
   {
      cv::minMaxLoc(mar, &mar_min, &mar_max);

      display_tau_min = mar_min * scale_factor;
      display_tau_max = mar_max * scale_factor;

      emit displayTauMinChanged(display_tau_min);
      emit displayTauMaxChanged(display_tau_max);
   }
   else
   {
      mar_min = display_tau_min / scale_factor;
      mar_max = display_tau_max / scale_factor;
   }

   mar.convertTo(scaled_mar, CV_8U, 255.0 / (mar_max - mar_min), -255.0 * mar_min / (mar_max - mar_min));

   cv::minMaxLoc(scaled_mar, &mar_min, &mar_max);


   cv::applyColorMap(scaled_mar, mapped_mar, cv::COLORMAP_JET);

   double i_min, i_max;
   if (autoscale_intensity && rescale)
   {
      cv::Scalar mean, std;
      cv::meanStdDev(intensity, mean, std);
      i_max = mean[0] + 1.96 * std[0]; // 97.5% 
      i_min = 0;

      if (display_intensity_min != i_min)
      {
         display_intensity_min = i_min;
         emit displayIntensityMinChanged(display_intensity_min);
      }
      if (display_intensity_max != i_max)
      {
         display_intensity_max = i_max;
         emit displayIntensityMaxChanged(display_intensity_max);
      }
   }
   else
   {
      i_min = display_intensity_min;
      i_max = display_intensity_max;
   }

   intensity.convertTo(alpha, CV_8U, 255.0 / (i_max - i_min), -255.0 * i_min / (i_max - i_min));

   display = cv::Mat(mapped_mar.rows, mapped_mar.cols, CV_8UC4);
   cv::mixChannels(std::vector<cv::Mat>({ mapped_mar, alpha }), { { display } }, { 0, 0, 1, 1, 2, 2, 3, 3 }); // to ARGB1

   lifetime_image_widget->SetImage(display);
}

void LifetimeDisplayWidget::setupPlots()
{
   decay_plot->xAxis->setLabel(QString("Time (%1)").arg(unit));
   decay_plot->yAxis->setLabel("Counts");
   decay_plot->yAxis->setScaleType(QCPAxis::stLogarithmic);

   decay_plot->replot();

	/*
   lifetime_histogram_plot->addGraph();
   lifetime_histogram_plot->xAxis->setLabel(QString("Lifetime (%1)").arg(unit));
   lifetime_histogram_plot->yAxis->setLabel("Frequency");
   lifetime_histogram_plot->replot();
   */
}
