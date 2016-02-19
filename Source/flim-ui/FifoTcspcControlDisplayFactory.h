#pragma once

#include "Cronologic.h"
#include "CronologicControlDisplay.h"
#include "SimTcspc.h"

class FifoTcspcControlDisplayFactory 
{
public:
   static QWidget* createControlDisplay(Cronologic* tcspc)
   {
      return new CronologicControlDisplay(tcspc);
   }

   static QWidget* createControlDisplay(SimTcspc* tcspc)
   {
      return new QWidget();
   }
};