#pragma once

#include "FlimReaderDataSource.h"
#include "IntensityReader.h"

#include <QObject>



class IntensityDataSource : public QObject, public AligningReader
{
   Q_OBJECT

public:

   IntensityDataSource(const QString& filename, QObject* parent = 0) : 
      QObject(parent), filename(filename)
   {
      reader = std::make_unique<IntensityReader>(filename.toStdString());
   }

   QString getFilename() { return filename; };
   void setRealignmentParameters(const RealignmentParameters& params) { AligningReader::setRealignmentParameters(params); };
   const std::unique_ptr<AbstractFrameAligner>& getFrameAligner() { return frame_aligner; };
   const std::vector<RealignmentResult>& getRealignmentResults() { return realignment; };

   void setReferenceIndex(int index) {};
   void readData(bool realign = true) {};
   void waitForComplete() {};
   void requestDelete() {};


private:

   std::unique_ptr<IntensityReader> reader;

   QString filename;
   int n_chan;
};

