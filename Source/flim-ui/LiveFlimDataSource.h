#pragma once
#include "FlimDataSource.h"
#include "FifoTcspc.h"

class LiveFlimDataSource : public FlimDataSource
{
   Q_OBJECT

public:
    
   LiveFlimDataSource(FifoTcspc* tcspc, QObject* parent = 0);
   std::shared_ptr<LiveFlimReader> getLiveReader() { return live_reader; }

   void reset();

protected:
   std::shared_ptr<LiveFlimReader> live_reader;
   FifoTcspc* tcspc;
};