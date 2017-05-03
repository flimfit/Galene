#pragma once
#include "ui_RealignmentDisplayWidget.h"
#include "ControlBinder.h"
#include "FlimReaderDataSource.h"
#include "qcustomplot.h"
#include <memory>
#include <QStandardPaths>

#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "WriteMultipageTiff.h"

class RealignmentDisplayWidget : public QWidget, public Ui::RealignmentDisplayWidget
{
   Q_OBJECT

public:

	RealignmentDisplayWidget(std::shared_ptr<FlimReaderDataSource> reader, QWidget* parent = 0);
	~RealignmentDisplayWidget();

   void exportMovie();

   void axisClicked(QCPAxis *  axis, QCPAxis::SelectablePart  part, QMouseEvent *  event);

protected:

   void update();
   void displayImage(int image);
   void axisClick(const QPointF& point);
   void setupPlots();

   std::shared_ptr<FlimReaderDataSource> reader;

   std::vector<QColor> line_colors;
   std::vector<QLabel*> rate_labels;

   std::vector<RealignmentResult> results;
};