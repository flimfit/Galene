#pragma once

#include "FlimReaderDataSource.h"
#include <QObject>

#include <ome/files/VariantPixelBuffer.h>
#include <ome/files/in/TIFFReader.h>

#include <Cv3dUtils.h>

int getCvPixelType(ome::xml::model::enums::PixelType pixel_type)
{
   switch(pixel_type)
   {
      case ome::xml::model::enums::PixelType::INT8: return CV_8S;
      case ome::xml::model::enums::PixelType::INT16: return CV_16S;
      case ome::xml::model::enums::PixelType::INT32: return CV_32S;
      case ome::xml::model::enums::PixelType::UINT8: return CV_8U;
      case ome::xml::model::enums::PixelType::UINT16: return CV_16U;
      case ome::xml::model::enums::PixelType::UINT32: return CV_32S; // OpenCV doesn't have CV_32U type
      case ome::xml::model::enums::PixelType::FLOAT: return CV_32F;
      case ome::xml::model::enums::PixelType::DOUBLE: return CV_64F;
      case ome::xml::model::enums::PixelType::COMPLEXFLOAT: return CV_32FC2;
      case ome::xml::model::enums::PixelType::COMPLEXDOUBLE: return CV_64FC2;
      default: throw std::runtime_error("unsupported data format");
   }
   return CV_8S;
}

class IntensityReader : public AligningReader
{
public:

   IntensityReader(const std::string& filename) : 
      filename(filename)
   {
      // Create TIFF reader
      reader = std::make_shared<ome::files::in::TIFFReader>();
         
      // Set reader options before opening a file
      reader->setMetadataFiltered(false);
      reader->setGroupFiles(true);

      // Open the file
      reader->setId(filename);
   }

   void getIntensityFrames() {


   };

   ImageScanParameters getImageScanParameters() { return scan_params; }

protected:

   void readMetadata()
   {
      // Get total number of images (series)
      auto ic = reader->getSeriesCount();
 
      if (ic > 1)
         std::cout << "Only reading first image";

      // Change the current series to this index
      reader->setSeries(0);
 
      scan_params = ImageScanParameters(100, 100, 0, reader->getSizeX(), reader->getSizeY(), reader->getSizeZ());

      n_t = reader->getSizeT();
   }

   void read()
   {
      // Get total number of planes (for this image index)
      int pc = reader->getImageCount();
      
      ome::files::VariantPixelBuffer buf;

      std::vector<int> dims = {scan_params.n_z, scan_params.n_y, scan_params.n_x};

      for(int i=0; i<n_t; i++)
         frames.push_back( cv::Mat(dims, CV_32F, cv::Scalar(0)) );

         cv::Mat cv32;

         // Loop over planes (for this image index)
      for (int p = 0 ; p < pc; ++p)
      {
         auto coords = reader->getZCTCoords(p);
         reader->openBytes(p, buf);

         // Create CV wrapper
         int type = getCvPixelType(buf.pixelType());
         cv::Mat cvbuf(scan_params.n_y, scan_params.n_x, type, buf.data());

         // Convert to 32F
         cvbuf.convertTo(cv32, CV_32F);

         // Copy into frame
         cv::Mat slice = extractSlice(frames[coords[2]], coords[0]);
         slice += cv32;
      }
   }

   ImageScanParameters scan_params;
   std::shared_ptr<ome::files::FormatReader> reader;   
   std::string filename;

   int n_t;
};

/*

class IntensityDataSource : public QObject, public AligningReader
{
   Q_OBJECT

public:

   IntensityDataSource(const QString& filename, QObject* parent = 0) : 
      QObject(parent), filename(filename)
   {
      // Create TIFF reader
      reader = std::make_shared<ome::files::in::TIFFReader>();
          
      // Set reader options before opening a file
      reader->setMetadataFiltered(false);
      reader->setGroupFiles(true);

      // Open the file
      reader->setId(filename.toStdString());
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

  

   std::vector<cv::Mat> frames;
   std::vector<int> dims;
   QString filename;
   int n_chan;
};


*/