#pragma once

#include "FifoTcspc.h"
#include "CronologicControlDisplay.h"
#include "SimTcspc.h"

class FifoTcspcControlDisplayFactory 
{
public:
   static QWidget* create(FifoTcspc* tcspc)
   {
#ifdef USE_CRONOLOGIC
      if (Cronologic* c = dynamic_cast<Cronologic*>(tcspc))
         return new CronologicControlDisplay(c);
#endif

      return new QWidget();
   }
};
