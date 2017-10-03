#include "RealignmentDisplayWidget.h"
#include "RealignmentResultsWriter.h"
#include "Cv3dUtils.h"

RealignmentDisplayWidget::RealignmentDisplayWidget(std::shared_ptr<RealignableDataSource> source, QWidget* parent) :
   QWidget(parent), source(source)
{
   setupUi(this);
   setupPlots();
   z_scroll->setVisible(false);

   connect(image_widget, &ImageRenderWidget::ConstrainWidth, this, &RealignmentDisplayWidget::setMaximumWidth);

   timer = new QTimer(this);
   connect(timer, &QTimer::timeout, this, &RealignmentDisplayWidget::update);
   timer->start(5000);
   
   //connect(source.get(), &FlimReaderDataSource::readComplete, this, &RealignmentDisplayWidget::update, Qt::QueuedConnection);
   connect(z_scroll, &QScrollBar::valueChanged, this, &RealignmentDisplayWidget::setZ);   
   connect(slider, &QSlider::valueChanged, this, &RealignmentDisplayWidget::setImage);
   connect(slider, &QSlider::sliderMoved, current_frame_spin, &QSpinBox::setValue);
   //connect(current_frame_spin, &QSpinBox::editingFinished, [&]() { slider->setValue(current_frame_spin->value()); });

   connect(set_reference_button, &QPushButton::pressed, this, &RealignmentDisplayWidget::referenceButtonPressed);
}

RealignmentDisplayWidget::~RealignmentDisplayWidget()
{
   //disconnect(source.get(), &FlimReaderDataSource::readComplete, this, &RealignmentDisplayWidget::update);
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

   bool use_aligned = show_aligned_button->isChecked();
   if (use_aligned)
      RealignmentResultsWriter::exportAlignedMovie(results, file);
   else
      RealignmentResultsWriter::exportUnalignedMovie(results, file);

}


void RealignmentDisplayWidget::update()
{
   results = source->getRealignmentResults();

   int n = (int)results.size();

   float minc = std::numeric_limits<float>::min();
   float maxc = std::numeric_limits<float>::min();
   QVector<double> x(n);
   QVector<double> y(n);

   for (int i = 0; i < n; i++)
   {
      x[i] = i;
      y[i] = results[i].correlation;

      if (results[i].correlation < minc)
         minc = results[i].correlation;
      if (results[i].correlation > maxc)
         maxc = results[i].correlation;
   }
   correlation_plot->graph(0)->setData(x, y);
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
   bool use_aligned = show_aligned_button->isChecked();

   if (cur_image < results.size())
   {
      cv::Mat sel_image = (use_aligned) ? results[cur_image].realigned : results[cur_image].frame;

      if (sel_image.dims > 2)
      {
         z_scroll->setVisible(sel_image.size[0] > 1);
         z_scroll->setMaximum(sel_image.size[0] - 1);
         int z = z_scroll->value();
         sel_image = extractSlice(sel_image, z);
      }

      image_widget->SetImage(sel_image);
      
      correlation_plot->graph(1)->setData({ (double)cur_image }, { results[cur_image].correlation });
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
   correlation_plot->xAxis->setLabel("Frame");
   correlation_plot->yAxis->setLabel("Correlation");

   correlation_plot->addGraph();
   correlation_plot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));
   correlation_plot->graph(1)->setPen(QPen(Qt::red));

   connect(correlation_plot, &QCustomPlot::axisClick, this, &RealignmentDisplayWidget::axisClicked);
}
