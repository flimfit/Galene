#pragma once

#include "FifoTcspc.h"
#include "SimTcspc.h"
#include "CronologicControlDisplay.h"
#include "SimTcspcControlDisplay.h"


class FifoTcspcControlDisplayFactory 
{
public:
   static QWidget* create(FifoTcspc* tcspc)
   {
#ifdef USE_CRONOLOGIC
      if (Cronologic* c = dynamic_cast<Cronologic*>(tcspc))
         return new CronologicControlDisplay(c);
#endif
      if (SimTcspc* c = dynamic_cast<SimTcspc*>(tcspc))
         return new SimTcspcControlDisplay(c);
      return new QWidget();
   }
};
