#pragma once
#include <QDir>
#include <QFileDialog>
#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QSettings>
#include <QStringList>

class FlimWorkspace : public QAbstractListModel
{
   Q_OBJECT

public:
   FlimWorkspace(QObject* parent = 0, const QString& dir = "") :
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
         QString last_workspace = settings.value("workspace/last_workspace","").toString();
         if (!last_workspace.isEmpty())
            openFromFolder(last_workspace);
      }
   }

   void update()
   {
      if (has_workspace)
      {
         beginResetModel();
         auto file_info = workspace.entryInfoList({ QString("*%1").arg(file_extension) }, QDir::NoFilter, QDir::Time | QDir::Reversed);
         files.clear();
         for (auto& info : file_info)
            files.append(info.baseName());
         endResetModel();
      }
   }

   bool hasWorkspace() { return has_workspace;  }

   int rowCount(const QModelIndex & parent = QModelIndex()) const
   {
      return files.length();
   }

   QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const
   {
      if (role == Qt::DisplayRole || role == Qt::EditRole)
      {
         if (index.row() < files.size())
            return files[index.row()];
      }

      return QVariant();
   }

   Qt::ItemFlags flags(const QModelIndex &index) const
   {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
   }

   bool setData(const QModelIndex &index, const QVariant &value, int role)
   {
      if (role == Qt::EditRole)
         return workspace.rename(index.data().toString().append(file_extension), value.toString().append(file_extension));
      return false;
   }


   void deleteFiles(const QModelIndexList& indexes, bool request_confirmation = true)
   {
      if (request_confirmation)
      {
         auto response = QMessageBox::question(nullptr, "Confirm File Deletion", "Are you sure you want to delete the selected files?");
         if (response == QMessageBox::StandardButton::No) return;
      }

      for (auto& idx : indexes)
         workspace.remove(idx.data().toString().append(file_extension));
   }

   void requestOpenFiles(const QModelIndexList& indexes)
   {
      for (auto& idx : indexes)
         requestOpenFile(idx);
   }

   void requestOpenFile(const QModelIndex index)
   {
      QString full_file = workspace.absoluteFilePath(QString("%1%2").arg(index.data().toString()).arg(file_extension));
      emit openRequest(full_file);
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

      setWorkspace(dir);
   }

   void openFromFolder(const QString& dir)
   {
      setWorkspace(dir);
   }


   QString getNextFileName()
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

   QString getFileName(const QString& filename)
   {
      if (!has_workspace)
         throw std::runtime_error("No workspace selected. Please select a workspace from the file menu.");

      return workspace.absoluteFilePath(filename).append(file_extension);
   }


   void setFilePrefix(const QString& prefix_)
   { 
      QString new_prefix = prefix_; 
      new_prefix.remove(QRegExp("[/\\\\:\\.""\\*<>\\?|]")); // attempt to remove illegal characters - this isn't comprehensive
      file_prefix = new_prefix;
      sequence_number = 1;
      emit sequenceNumberChanged(sequence_number);
   }
   
   const QString& getFilePrefix() { return file_prefix; }

   void setSequenceNumber(int sequence_number_) { sequence_number = sequence_number_; }
   int getSequenceNumber() { return sequence_number; }

signals:
   void sequenceNumberChanged(int sequence_number);
   void openRequest(const QString& file);

protected:

   void setWorkspace(const QString& dir)
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


   bool has_workspace = false;
   QDir workspace;
   QString file_prefix = "FLIM_Data_";
   int sequence_number = 0;
   QString file_extension = ".ffd";

   int n_files;
   QStringList files;

   QFileSystemWatcher* folder_watcher;
};


class WorkspaceEventFilter : public QObject
{
   Q_OBJECT

public:
   WorkspaceEventFilter(FlimWorkspace* workspace) :
      workspace(workspace)
   {

   }


protected: 
   bool eventFilter(QObject *obj, QEvent* event)
   {
      QListView* view = reinterpret_cast<QListView*>(obj);

      if (event->type() == QEvent::KeyPress)
      {
         QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
         
         if (key_event->matches(QKeySequence::Delete))
         {
            deleteSelected(obj);
            return true; 
         }
      }
      else if (event->type() == QEvent::ContextMenu)
      {
         QContextMenuEvent *menu_event = static_cast<QContextMenuEvent*>(event);

         int n_sel = selected(obj).length();

         if (n_sel == 0)
            return true;

         QMenu menu(view);
         
         menu.addAction("Open", [this, obj]() { requestOpenSelected(obj); });
         menu.addAction("Delete...", [this, obj]() { deleteSelected(obj); }, QKeySequence::StandardKey::Delete);
         
         if (n_sel == 1)
            menu.addAction("Rename...", [this, obj]() { // this is a bit of a nasty fudge! must be a better way...
            QKeyEvent evt(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier);
            QCoreApplication::sendEvent(obj, &evt);
         }, QKeySequence("F2"));

         menu.exec(menu_event->globalPos());
      }

      return QObject::eventFilter(obj, event);
      
   }

   void deleteSelected(QObject* obj)
   {
      workspace->deleteFiles(selected(obj));
   }

   QModelIndexList selected(QObject* obj)
   {
      QListView* view = reinterpret_cast<QListView*>(obj);
      return view->selectionModel()->selectedRows();
   }

   void requestOpenSelected(QObject* obj)
   {
      workspace->requestOpenFiles(selected(obj));
   }

   FlimWorkspace* workspace;
};