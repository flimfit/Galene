#pragma once

#include "RealignableDataSource.h"
#include "IntensityReader.h"

#include <QObject>



class IntensityDataSource : public QObject, public RealignableDataSource
{
   Q_OBJECT

signals:
   void error(const QString&);

public:

   IntensityDataSource(const QString& filename, QObject* parent = 0) : 
      QObject(parent), filename(filename)
   {
      reader = std::make_unique<IntensityReader>(filename.toStdString());
   }

   QString getFilename() { return filename; };
   
   void setupForRead();
   void update();
   void alignFrames();
   void readAlignedData();

   void cancelRead();
   
   void requestDelete() {};

protected:

   AligningReader& aligningReader() { return *reader; };
   

private:

   std::unique_ptr<IntensityReader> reader;

   QString filename;
   int n_chan;
};

