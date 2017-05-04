
#include "SimTcspcControlDisplay.h"

#include <QApplication>
#include <QInputDialog>
#include <QErrorMessage>
#include <QGroupBox>

#include <iostream>


SimTcspcControlDisplay::SimTcspcControlDisplay(SimTcspc* tcspc, QWidget* parent) :
   tcspc(tcspc), QWidget(parent)
{

   // Threshold Controls
   //===================================================
   QFormLayout* motion_layout = new QFormLayout();
   addWidget(motion_layout, "DisplacementAmplitude", Float);
   addWidget(motion_layout, "DisplacementFrequency", Float);

   QGroupBox* thresh_group = new QGroupBox("Motion");
   thresh_group->setLayout(motion_layout);


   // Add control panels to window
   //==================================================
   QVBoxLayout* side_layout = new QVBoxLayout();
   side_layout->addWidget(thresh_group);
   side_layout->setMargin(0);

   setLayout(side_layout);
}