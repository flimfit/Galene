#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QRegularExpression>
#include <QFileInfo>
#include <opencv2/opencv.hpp>
#include "BioImageIntensityReader.h"

class SpectralCorrectionListModel : public QAbstractListModel
{
public:

   SpectralCorrectionListModel(QObject* parent = nullptr);

   int rowCount(const QModelIndex& parent = QModelIndex()) const { return files.size(); }
   QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
   
   void remove(const QModelIndexList& indexes);
   void add(const QStringList& new_files);
   std::vector<cv::Mat> getMatching(const QString& filename);


private:
   QList<QString> files;
   std::map<QString, std::vector<cv::Mat>> correction;

   QRegularExpression re;
};