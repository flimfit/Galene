#ifdef USE_CRONOLOGIC
#include "CronologicControlDisplay.h"

#include <QApplication>
#include <QInputDialog>
#include <QErrorMessage>
#include <QGroupBox>

#include <iostream>


CronologicControlDisplay::CronologicControlDisplay(Cronologic* tcspc, QWidget* parent) :
   tcspc(tcspc), QWidget(parent)
{

   // Threshold Controls
   //===================================================
   QFormLayout* thresh_layout = new QFormLayout();
   addWidget(thresh_layout, "Start_Threshold", Float);
   addWidget(thresh_layout, "Threshold_0", Float);
   addWidget(thresh_layout, "Threshold_1", Float);
   addWidget(thresh_layout, "Threshold_2", Float);

   QGroupBox* thresh_group = new QGroupBox("Thresholds");
   thresh_group->setLayout(thresh_layout);

   // Threshold Controls
   //===================================================
   QFormLayout* time_shift_layout = new QFormLayout();
   addWidget(time_shift_layout, "Time_Shift_0", Integer);
   addWidget(time_shift_layout, "Time_Shift_1", Integer);
   addWidget(time_shift_layout, "Time_Shift_2", Integer);

   QGroupBox* time_shift_group = new QGroupBox("Time Shifts");
   time_shift_group->setLayout(time_shift_layout);

   // PLIM Controls
   QFormLayout* plim_layout = new QFormLayout();
   addWidget(plim_layout, "Num_Pixel_PLIM", Integer);

   QGroupBox* plim_group = new QGroupBox("PLIM Settings");
   plim_group->setLayout(plim_layout);


   // Add control panels to window
   //==================================================
   QVBoxLayout* side_layout = new QVBoxLayout();
   side_layout->addWidget(thresh_group);
   side_layout->addWidget(time_shift_group);
   side_layout->addWidget(plim_group);
   side_layout->setMargin(0);

   setLayout(side_layout);
}

#endif