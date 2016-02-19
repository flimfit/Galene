#include "CronologicControlDisplay.h"

#include <QApplication>
#include <QInputDialog>
#include <QErrorMessage>
#include <QGroupBox>

#include <iostream>



CronologicControlDisplay::CronologicControlDisplay(Cronologic* tcspc, QWidget* parent) :
   tcspc(tcspc), QWidget(parent)
{

   // ROI Controls
   //===================================================
   QFormLayout* thresh_layout = new QFormLayout();
   addWidget(thresh_layout, "Threshold_0", Integer);
   addWidget(thresh_layout, "Threshold_1", Integer);
   addWidget(thresh_layout, "Threshold_2", Integer);

   QGroupBox* thresh_group = new QGroupBox("Thresholds");
   thresh_group->setLayout(thresh_layout);


   // Add control panels to window
   //==================================================
   QVBoxLayout* side_layout = new QVBoxLayout();
   side_layout->addWidget(thresh_group);
   side_layout->setMargin(0);

   setLayout(side_layout);
}

