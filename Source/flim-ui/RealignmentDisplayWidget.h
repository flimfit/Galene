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

	RealignmentDisplayWidget(std::shared_ptr<RealignableDataSource> source, QWidget* parent = 0);
	~RealignmentDisplayWidget();

   void exportMovie();

   void axisClicked(QCPAxis *  axis, QCPAxis::SelectablePart  part, QMouseEvent *  event);

signals:

   void referenceIndexUpdated(int reference_index);

protected:

   void update();
   void setImage(int image);
   void setZ(int z);
   void axisClick(const QPointF& point);
   void setupPlots();
   void referenceButtonPressed();

   void drawImage();
   
   std::shared_ptr<RealignableDataSource> source;

   std::vector<QColor> line_colors;
   std::vector<QLabel*> rate_labels;

   std::vector<RealignmentResult> results;

   QTimer* timer;

   int cur_image = 0;
};