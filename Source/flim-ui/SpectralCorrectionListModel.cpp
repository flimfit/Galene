#include "SpectralCorrectionListModel.h"
#include <QMessageBox>
#include <bim_image_stack.h>

SpectralCorrectionListModel::SpectralCorrectionListModel(QObject* parent) :
      QAbstractListModel(parent)
   {
      re = QRegularExpression("zoom=\\d+(-\\d+)*");
   }

   QVariant SpectralCorrectionListModel::data(const QModelIndex& index, int role) const
   {
      if (role == Qt::DisplayRole)
         return files.at(index.row());
      return QVariant();
   }

   void SpectralCorrectionListModel::remove(const QModelIndexList& indexes)
   {
      int offset = 0;
      std::vector<int> rows(indexes.size());
      for (int i = 0; i < indexes.size(); i++)
         rows[i] = indexes[i].row();
      std::sort(rows.rbegin(), rows.rend());
      for (auto r : rows)
      {
         correction.erase(files[r]);
         files.removeAt(r);
      }
      emit dataChanged(index(0, 0), index(files.size(), 0));
   }

   void SpectralCorrectionListModel::add(const QStringList& new_files)
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

         auto stack = bim::ImageStack();
         stack.fromFile(f.toStdString());
         for(int i=0; i<stack.numberPlanes(); i++)
         {
            auto im = stack.imageAt(i)->toCVMat(0);
            mats.push_back(im);
         }
         correction[zoom] = mats;
      }
      emit dataChanged(index(start_row, 0), index(files.size(), 0));
   }

   std::vector<cv::Mat> SpectralCorrectionListModel::getMatching(const QString& filename)
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