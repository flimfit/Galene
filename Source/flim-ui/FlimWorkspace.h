#pragma once
#include <QDir>
#include <QFileDialog>
#include <QAbstractListModel>
#include <QFileSystemWatcher>

class FlimWorkspace : public QAbstractListModel
{
   Q_OBJECT

public:
   FlimWorkspace(QObject* parent = 0, const QString& dir = "") :
      QAbstractListModel(parent)
   {
      if (dir != "")
      {
         if (QDir(dir).exists())
            openFromFolder(dir);
         else
            makeNewFromFolder(dir);
      }

      folder_watcher = new QFileSystemWatcher(this);
      connect(folder_watcher, &QFileSystemWatcher::directoryChanged, this, &FlimWorkspace::update);
   }

   void update()
   {
      if (has_workspace)
      {
         beginResetModel();
         files = workspace.entryList({ "*.ffd" }, QDir::NoFilter, QDir::Time | QDir::Reversed);
         endResetModel();
      }
   }

   int rowCount(const QModelIndex & parent = QModelIndex()) const
   {
      return files.length();
   }

   QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const
   {
      if (role == Qt::DisplayRole)
      {
         if (index.row() < files.size())
            return files[index.row()];
      }

      return QVariant();
   }

   void makeNew()
   {
      QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
      QString new_dir = QFileDialog::getSaveFileName(nullptr, "Choose a workspace", folder, "");

      makeNewFromFolder(new_dir);
   }

   void open()
   {
      QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
      QString open_dir = QFileDialog::getExistingDirectory(nullptr, "Choose a workspace", folder);

      openFromFolder(open_dir);
   }

   void makeNewFromFolder(const QString& dir)
   {
      if (!QDir(dir).exists())
         QDir().mkdir(dir);

      folder_watcher->removePath(workspace.absolutePath());
      folder_watcher->addPath(dir);

      workspace = dir;
      has_workspace = true;
      update();
   }

   void openFromFolder(const QString& dir)
   {
      folder_watcher->removePath(workspace.absolutePath());
      folder_watcher->addPath(dir);

      workspace = dir;
      has_workspace = true;
      update();
   }

   QString getNextFileName()
   {
      if (!has_workspace)
         throw std::runtime_error("No workspace selected. Please select a workspace from the file menu.");

      QString next_file = workspace.absoluteFilePath(QString("%1_%2.ffd").arg(file_prefix).arg(sequence_number, 3, 10, QChar('0')));

      emit sequenceNumberChanged(++sequence_number);

      return next_file;
   }

   void setFilePrefix(const QString& prefix_) { file_prefix = prefix_; }
   const QString& getFilePrefix() { return file_prefix; }

   void setSequenceNumber(int sequence_number_) { sequence_number = sequence_number_; }
   int getSequenceNumber() { return sequence_number; }

signals:
   void sequenceNumberChanged(int sequence_number);

protected:

   void enumerate()
   {

   }

   bool has_workspace = false;
   QDir workspace;
   QString file_prefix = "FLIM_Data_";
   int sequence_number = 0;

   int n_files;
   QStringList files;

   QFileSystemWatcher* folder_watcher;
};
