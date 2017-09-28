#pragma once

#include <QWidget>
#include <QString>
#include <memory>
#include <vector>

class RealignableDataSource
{
public:
   
   virtual QString getFilename() = 0;
   virtual void setRealignmentParameters(const RealignmentParameters& params) = 0;
   virtual const std::unique_ptr<AbstractFrameAligner>& getFrameAligner() = 0;
   virtual const std::vector<RealignmentResult>& getRealignmentResults() = 0;

   virtual void setReferenceIndex(int index) = 0;
   virtual void readData(bool realign = true) = 0;
   virtual void waitForComplete() = 0;
   virtual void requestDelete() = 0;

   virtual QWidget* getWidget() { return nullptr; }
};
