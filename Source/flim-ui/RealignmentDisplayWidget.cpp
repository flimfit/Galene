#include "RealignmentDisplayWidget.h"
#include "RealignmentResultsWriter.h"

RealignmentDisplayWidget::RealignmentDisplayWidget(std::shared_ptr<FlimReaderDataSource> reader, QWidget* parent) :
   QWidget(parent), reader(reader)
{
   setupUi(this);
   setupPlots();
   updateGeometry();
   connect(image_widget, &ImageRenderWidget::ConstrainWidth, this, &RealignmentDisplayWidget::setMaximumWidth);

   connect(reader.get(), &FlimReaderDataSource::readComplete, this, &RealignmentDisplayWidget::update);
   connect(slider, &QSlider::valueChanged, this, &RealignmentDisplayWidget::displayImage);
}

RealignmentDisplayWidget::~RealignmentDisplayWidget()
{
   disconnect(reader.get(), &FlimReaderDataSource::readComplete, this, &RealignmentDisplayWidget::update);
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
   auto r = reader->getReader();
   results = r->getRealignmentResults();

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

   slider->setMaximum(n - 1);
   slider->setValue(0);

   displayImage(0);
}

void RealignmentDisplayWidget::displayImage(int image)
{
   bool use_aligned = show_aligned_button->isChecked();
   if (image < results.size())
   {
      if (use_aligned)
         image_widget->SetImage(results[image].realigned);
      else
         image_widget->SetImage(results[image].frame);

      correlation_plot->graph(1)->setData({ (double)image }, { results[image].correlation });
      correlation_plot->yAxis->setRange(0, 1.2);
      //correlation_plot->replot();
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
