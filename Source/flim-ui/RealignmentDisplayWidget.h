#pragma once
#include "ui_RealignmentDisplayWidget.h"
#include "ControlBinder.h"
#include "FlimReaderDataSource.h"
#include "qcustomplot.h"
#include <memory>
#include <QStandardPaths>

#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>

class RealignmentDisplayWidget : public QWidget, public Ui::RealignmentDisplayWidget
{
   Q_OBJECT

public:

   RealignmentDisplayWidget(std::shared_ptr<FlimReaderDataSource> reader, QWidget* parent = 0) :
      QWidget(parent), reader(reader)
   {
      setupUi(this);
      setupPlots();
      updateGeometry();
      connect(image_widget, &ImageRenderWidget::ConstrainWidth, this, &RealignmentDisplayWidget::setMaximumWidth);

      connect(reader.get(), &FlimReaderDataSource::alignmentComplete, this, &RealignmentDisplayWidget::update);
      connect(slider, &QSlider::valueChanged, this, &RealignmentDisplayWidget::displayImage);
   }

   void exportMovie()
   {
      if (results.empty())
      {
         QMessageBox::warning(this, "Could not export", "No results to export from this window");
         return;
      }

      QString file = QFileDialog::getSaveFileName(this, "Choose file name", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "*.avi");

      if (file.isEmpty()) return;

      auto sz = results[0].frame.size();
      auto writer = cv::VideoWriter(file.toStdString(), -1, 4, sz, false);

      bool use_aligned = show_aligned_button->isChecked();

      double mn, mx;
      cv::minMaxIdx(results[1].frame, &mn, &mx);
      double scale = 255 / (0.8*mx);

      cv::Mat buf(sz, CV_8U);
      for (const auto& r : results)
      {
         if (use_aligned)
            r.realigned.convertTo(buf, CV_8U, scale);
         else
            r.frame.convertTo(buf, CV_8U, scale);
         writer.write(buf);
      }
   }


signals:

protected:

   std::shared_ptr<FlimReaderDataSource> reader;

   void update()
   {
      auto r = reader->getReader();
      results = r->getRealignmentResults();

      int n = (int) results.size();

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
      correlation_plot->update();
      
      slider->setMaximum(n-1);
      slider->setValue(0);
      
      displayImage(0);
   }

   void displayImage(int image)
   {
      bool use_aligned = show_aligned_button->isChecked();
      if (image < results.size())
      {
         if (use_aligned)
            image_widget->SetImage(results[image].realigned);
         else
            image_widget->SetImage(results[image].frame);

         correlation_plot->graph(1)->setData({ (double)image }, { results[image].correlation });
         correlation_plot->update();
      }
   }

   void axisClick(const QPointF& point)
   {
      int p = (int) std::round(point.x());
      if (p >= 0 && p <= slider->maximum())
         slider->setValue(p);
   }

   void setupPlots()
   {
      
      correlation_plot->addGraph();
      correlation_plot->graph(0)->setPen(QPen(Qt::black));
      correlation_plot->xAxis->setLabel("Frame");
      correlation_plot->yAxis->setLabel("Correlation");

      correlation_plot->addGraph();
      correlation_plot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));
      correlation_plot->graph(1)->setPen(QPen(Qt::red));
     
   }

   std::vector<QColor> line_colors;
   std::vector<QLabel*> rate_labels;

   std::vector<RealignmentResult> results;
};