#include "FlimReaderDataSource.h"
#include <QObject>

class IntensityDataSource : public QObject, public RealignableDataSource
{
public:

   IntensityDataSource(const QString& filename, QObject* parent) : 
      QObject(parent)
   {

   }

private:
   std::vector<cv::Mat> frames;
}