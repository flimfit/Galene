#include "FlimWorkspace.h"

#include <QMessageBox>
#include <QStandardPaths>

FlimWorkspace::FlimWorkspace(QObject* parent, const QString& dir) :
   QAbstractListModel(parent)
{
   folder_watcher = new QFileSystemWatcher(this);
   connect(folder_watcher, &QFileSystemWatcher::directoryChanged, this, &FlimWorkspace::update);

   if (dir != "")
   {
      if (QDir(dir).exists())
         openFromFolder(dir);
      else
         makeNewFromFolder(dir);
   }
   else
   {
      QSettings settings;
      QString last_workspace = settings.value("workspace/last_workspace", "").toString();
      if (!last_workspace.isEmpty())
         openFromFolder(last_workspace);
   }
}

void FlimWorkspace::update()
{
   if (has_workspace)
   {
      beginResetModel();
      auto file_info = workspace.entryInfoList({ QString("*%1").arg(file_extension), QString("*.pt3"), QString("*.ffh") }, QDir::NoFilter, QDir::Time | QDir::Reversed);
      files.clear();
      for (auto& info : file_info)
         files.append(info.fileName());
      endResetModel();
   }
}


int FlimWorkspace::rowCount(const QModelIndex & parent) const
{
   return files.length();
}

QVariant FlimWorkspace::data(const QModelIndex & index, int role) const
{
   if (role == Qt::DisplayRole || role == Qt::EditRole)
   {
      if (index.row() < files.size())
         return files[index.row()];
   }

   return QVariant();
}

Qt::ItemFlags FlimWorkspace::flags(const QModelIndex &index) const
{
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool FlimWorkspace::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (role == Qt::EditRole)
      return workspace.rename(index.data().toString(), value.toString());
   return false;
}


void FlimWorkspace::deleteFiles(const QModelIndexList& indexes, bool request_confirmation)
{
   if (request_confirmation)
   {
      auto response = QMessageBox::question(nullptr, "Confirm File Deletion", "Are you sure you want to delete the selected files?");
      if (response == QMessageBox::StandardButton::No) return;
   }

   for (auto& idx : indexes)
      workspace.remove(idx.data().toString());
}

void FlimWorkspace::requestOpenFiles(const QModelIndexList& indexes)
{
   for (auto& idx : indexes)
      requestOpenFile(idx);
}

void FlimWorkspace::requestOpenFile(const QModelIndex index)
{
   QString full_file = workspace.absoluteFilePath(index.data().toString());
   emit openRequest(full_file);
}


const QString FlimWorkspace::getNewPath()
{
   if (has_workspace)
      return workspace.canonicalPath();
   else
      return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void FlimWorkspace::makeNew()
{
   QString new_dir = QFileDialog::getSaveFileName(nullptr, "Choose a workspace", getNewPath(), "");

   if (!new_dir.isEmpty())
     makeNewFromFolder(new_dir);
}

void FlimWorkspace::open()
{
   QString open_dir = QFileDialog::getExistingDirectory(nullptr, "Choose a workspace", getNewPath());

   if (!open_dir.isEmpty())
      openFromFolder(open_dir);
}

void FlimWorkspace::makeNewFromFolder(const QString& dir)
{
   if (!QDir(dir).exists())
      QDir().mkdir(dir);

   setWorkspace(dir);
}

void FlimWorkspace::openFromFolder(const QString& dir)
{
   setWorkspace(dir);
}


QString FlimWorkspace::getNextFileName()
{
   if (!has_workspace)
      throw std::runtime_error("No workspace selected. Please select a workspace from the file menu.");

   QString next_file;
   do
   {
      next_file = workspace.absoluteFilePath(QString("%1_%2%3").arg(file_prefix).arg(sequence_number, 3, 10, QChar('0')).arg(file_extension));
      sequence_number++;
   } while (workspace.exists(next_file));

   emit sequenceNumberChanged(sequence_number);

   return next_file;
}

QString FlimWorkspace::getFileName(const QString& filename, const QString& ext)
{
   if (!has_workspace)
      throw std::runtime_error("No workspace selected. Please select a workspace from the file menu.");

   if (ext.isEmpty())
      return workspace.absoluteFilePath(filename).append(file_extension);
   else
      return workspace.absoluteFilePath(filename).append(ext);

}

QStringList FlimWorkspace::getSelectedFiles(const QModelIndexList& indexes)
{
   QStringList files;

   for (auto& idx : indexes)
      files.push_back(workspace.absoluteFilePath(idx.data().toString()));

   return files;
}

void FlimWorkspace::setFilePrefix(const QString& prefix_)
{
   QString new_prefix = prefix_;
   new_prefix.remove(QRegExp("[/\\\\:\\.""\\*<>\\?|]")); // attempt to remove illegal characters - this isn't comprehensive
   file_prefix = new_prefix;
   sequence_number = 1;
   emit sequenceNumberChanged(sequence_number);
}


void FlimWorkspace::setWorkspace(const QString& dir)
{
   folder_watcher->removePath(workspace.absolutePath());
   folder_watcher->addPath(dir);

   workspace = dir;
   has_workspace = true;
   update();

   QSettings settings;
   settings.setValue("workspace/last_workspace", dir);

   QStringList recent_workspaces = settings.value("workspace/recent_workspaces").toStringList();
   recent_workspaces.push_front(dir);
   while (recent_workspaces.size() > 20)
      recent_workspaces.pop_back();
   recent_workspaces.removeDuplicates();
   settings.setValue("workspace/recent_workspaces", recent_workspaces);
}


