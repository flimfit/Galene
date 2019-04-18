#include "LiveFlimDataSource.h"



LiveFlimDataSource::LiveFlimDataSource(FifoTcspc* tcspc, QObject* parent) : 
   FlimDataSource(parent)
{

   auto params = tcspc->getAcquisitionParameters();
   live_reader = std::make_shared<LiveFlimReader>(params, this);
   reader = live_reader;

   tcspc->addTcspcEventConsumer(live_reader);

   init();
}
