#include "RealignmentDisplayWidget.h"
#include "RealignmentResultsWriter.h"
#include "Cv3dUtils.h"

RealignmentDisplayWidget::RealignmentDisplayWidget(std::shared_ptr<RealignableDataSource> source, QWidget* parent) :
   QWidget(parent), source(source)
{
   setupUi(this);
   setupPlots();
   z_scroll_frame->setVisible(false);

   connect(image_widget, &ImageRenderWidget::ConstrainWidth, this, &RealignmentDisplayWidget::setMaximumWidth);

   timer = new QTimer(this);
   connect(timer, &QTimer::timeout, this, &RealignmentDisplayWidget::update);
   timer->start(2000);
   
   connect(z_scroll, &QScrollBar::valueChanged, this, &RealignmentDisplayWidget::setZ);   
   connect(slider, &QSlider::valueChanged, this, &RealignmentDisplayWidget::setImage);
   connect(slider, &QSlider::valueChanged, current_frame_spin, &QSpinBox::setValue);
   connect(current_frame_spin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), 
      [&](int value) { if (slider->value() != value) slider->setValue(value); });

   connect(display_combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RealignmentDisplayWidget::drawImage);
   connect(set_reference_button, &QPushButton::pressed, this, &RealignmentDisplayWidget::referenceButtonPressed);
}

RealignmentDisplayWidget::~RealignmentDisplayWidget()
{
}

void RealignmentDisplayWidget::referenceButtonPressed()
{
   emit referenceIndexUpdated(slider->value());
}


void RealignmentDisplayWidget::exportMovie()
{
   if (results.empty())
   {
      QMessageBox::warning(this, "Could not export", "No results to export from this window");
      return;
   }

   QString file = QFileDialog::getSaveFileName(this, "Choose file name", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "AVI (*.avi);; Tiff Stack (*.tif)");
   if (file.isEmpty()) return;

   CachedMat RealignmentResult::*field;
   switch (display_combo->currentIndex())
   {
   case 0: field = &RealignmentResult::frame; break;
   case 1: field = &RealignmentResult::realigned_preserving; break;
   case 2: field = &RealignmentResult::realigned; break;
   }
   
   RealignmentResultsWriter::exportMovie(results, file, field);
}


void RealignmentDisplayWidget::update()
{
   results = source->getRealignmentResults();

   int n = (int)results.size();

   float minc = std::numeric_limits<float>::min();
   float maxc = std::numeric_limits<float>::min();
   QVector<double> x(n);
   QVector<double> y1(n), y2(n);

   for (auto it = results.begin(); it != results.end(); it++)
   {
      int i = (int) it->first;
      if (i >= n) continue;

      auto& r = it->second;
      x[i] = i;
      y1[i] = r.correlation;
      y2[i] = r.unaligned_correlation;

      if (r.correlation < minc)
         minc = r.correlation;
      if (r.correlation > maxc)
         maxc = r.correlation;
   }
   correlation_plot->graph(0)->setData(x, y1);
   correlation_plot->graph(1)->setData(x, y2);
   correlation_plot->graph(0)->rescaleAxes();
   correlation_plot->yAxis->setRange(0, 1.2);
   //correlation_plot->replot();

   slider->setMaximum(std::max(0,n - 1));

   if (cur_image <= (n-1))
   {
      slider->setValue(cur_image);
      current_frame_spin->setValue(cur_image);
   }

   drawImage();
}

void RealignmentDisplayWidget::setZ(int z)
{
   drawImage();
}

void RealignmentDisplayWidget::setImage(int image)
{
   cur_image = image;
   drawImage();
}

void RealignmentDisplayWidget::drawImage()
{
   auto it = results.find(cur_image);
   if (it != results.end())
   {
      auto& r = it->second;

      if (!r.done) return;

      cv::Mat sel_image;
      switch (display_combo->currentIndex())
      {
      case 0: sel_image = r.frame->get(); break;
      case 1: sel_image = r.realigned_preserving->get(); break;
      case 2: sel_image = r.realigned->get(); break;
      }

      if (sel_image.dims > 2)
      {
         z_scroll_frame->setVisible(sel_image.size[0] > 1);
         z_scroll->setMaximum(sel_image.size[0] - 1);
         int z = z_scroll->value();
         sel_image = extractSlice(sel_image, z);
      }

      image_widget->SetImage(sel_image);
      
      correlation_plot->graph(2)->setData({ (double)cur_image }, { r.correlation });
      correlation_plot->yAxis->setRange(0, 1.2);
      correlation_plot->replot();
   }
}

/*
void RealignmentDisplayWidget::axisClick(const QPointF& point)
{
   int p = (int)std::round(point.x());
   if (p >= 0 && p <= slider->maximum())
      slider->setValue(p);
}
*/

void RealignmentDisplayWidget::axisClicked(QCPAxis *  axis, QCPAxis::SelectablePart  part, QMouseEvent *  event)
{
   auto point = event->pos();
   
   int p = (int)std::round(point.x());
   if (p >= 0 && p <= slider->maximum())
      slider->setValue(p);

}

void RealignmentDisplayWidget::setupPlots()
{
   correlation_plot->addGraph();
   correlation_plot->graph(0)->setPen(QPen(Qt::black));
   correlation_plot->graph(0)->setName("Aligned");
   correlation_plot->addGraph();
   correlation_plot->graph(1)->setPen(QPen(Qt::red));
   correlation_plot->graph(1)->setName("Unaligned");

   correlation_plot->xAxis->setLabel("Frame");
   correlation_plot->yAxis->setLabel("Correlation");

   correlation_plot->addGraph();
   correlation_plot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));
   correlation_plot->graph(2)->setPen(QPen(Qt::red));
   correlation_plot->graph(2)->removeFromLegend();

   /*
   QFont legendFont = font();
   legendFont.setPointSize(8);
   correlation_plot->legend->setFont(legendFont);
   correlation_plot->legend->setSelectedFont(legendFont);
   correlation_plot->legend->setBorderPen(Qt::PenStyle::NoPen);
   correlation_plot->legend->setRowSpacing(2);
   correlation_plot->legend->setMargins(QMargins(0, 0, 0, 0));
   correlation_plot->legend->setBrush(Qt::BrushStyle::NoBrush);
   correlation_plot->legend->setVisible(true);
   */

   connect(correlation_plot, &QCustomPlot::axisClick, this, &RealignmentDisplayWidget::axisClicked);
}
