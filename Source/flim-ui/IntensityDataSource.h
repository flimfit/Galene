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

   IntensityDataSource(const QString& filename, QObject* parent = 0);
   
   QString getFilename() { return filename; };
   
   void saveData(const QString& filename, bool interpolate = false);
   void writeRealignmentMovies(const QString& filename_root) {} // no point writing these for intensity
   void setupForRead();
   void update();
   void alignFrames();
   void readAlignedData();

   void cancelRead();
   
   void requestDelete() {};

protected:

   AligningReader& aligningReader() { return *reader; };
   

private:

   std::shared_ptr<IntensityReader> reader;

   QString filename;
   int n_chan;
};

