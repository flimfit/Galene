#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QRegularExpression>
#include <QFileInfo>
#include <opencv2/opencv.hpp>

class SpectralCorrectionListModel : public QAbstractListModel
{
public:

   SpectralCorrectionListModel(QObject* parent = nullptr) :
      QAbstractListModel(parent)
   {
      re = QRegularExpression("zoom=\\d+(-\\d+)*");
   }

   int rowCount(const QModelIndex& parent = QModelIndex()) const { return files.size(); }

   QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
   {
      if (role == Qt::DisplayRole)
         return files.at(index.row());
      return QVariant();
   }

   void remove(const QModelIndexList& indexes)
   {
      int offset = 0;
      std::vector<int> rows(indexes.size());
      for (int i = 0; i < indexes.size(); i++)
         rows[i] = indexes[i].row();
      std::sort(rows.rbegin(), rows.rend());
      for (auto r : rows)
         files.removeAt(r);
      emit dataChanged(index(0, 0), index(files.size(), 0));
   }

   void add(const QStringList& new_files)
   {
      int start_row = files.size();
      for (auto& f : new_files)
      {
         QString filename = QFileInfo(f).fileName();
         auto match = re.match(filename);

         QString zoom;
         if (match.hasMatch())
            zoom = match.captured(0);
         else
            zoom = filename;

         files.push_back(zoom);
         std::vector<cv::Mat> mats;
         cv::imreadmulti(f.toStdString(), mats, cv::IMREAD_UNCHANGED);
         correction[zoom] = mats;
      }
      emit dataChanged(index(start_row, 0), index(files.size(), 0));
   }

   std::vector<cv::Mat> getMatching(const QString& filename)
   {
      for (auto it = correction.begin(); it != correction.end(); it++)
         if (filename.contains(it->first))
            return it->second;

      // If we only have one loaded, use that
      if (correction.size() == 1)
         return correction.begin()->second;
      if (correction.size() > 1)
         QMessageBox::warning(nullptr, "Warning", "No spectral correction file matched");

      return std::vector<cv::Mat>();
   }


private:
   QList<QString> files;
   std::map<QString, std::vector<cv::Mat>> correction;

   QRegularExpression re;
};