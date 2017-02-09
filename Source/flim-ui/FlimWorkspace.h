#pragma once
#include <QDir>
#include <QCoreApplication>
#include <QFileDialog>
#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QSettings>
#include <QStringList>
#include <QListView>
#include <QEvent>
#include <QMenu>
#include <QKeyEvent>

class FlimWorkspace : public QAbstractListModel
{
   Q_OBJECT

public:
   FlimWorkspace(QObject* parent = 0, const QString& dir = "");

   void update();

   bool hasWorkspace() { return has_workspace;  }

   int rowCount(const QModelIndex & parent = QModelIndex()) const;
   QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
   Qt::ItemFlags flags(const QModelIndex &index) const;
   bool setData(const QModelIndex &index, const QVariant &value, int role);

   void deleteFiles(const QModelIndexList& indexes, bool request_confirmation = true);
   void requestOpenFiles(const QModelIndexList& indexes);
   void requestOpenFile(const QModelIndex index);
   void makeNew();

   void open();
   void makeNewFromFolder(const QString& dir);
   void openFromFolder(const QString& dir);

   QString getNextFileName();
   QString getFileName(const QString& filename, const QString& ext = "");
   
   QStringList getSelectedFiles(const QModelIndexList& indexes);

   void setFilePrefix(const QString& prefix_);
   const QString& getFilePrefix() { return file_prefix; }

   void setSequenceNumber(int sequence_number_) { sequence_number = sequence_number_; }
   int getSequenceNumber() { return sequence_number; }

signals:
   void sequenceNumberChanged(int sequence_number);
   void openRequest(const QString& file);

protected:

   void setWorkspace(const QString& dir);
   const QString getNewPath();

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
   WorkspaceEventFilter(FlimWorkspace* workspace, bool can_process = false) :
      workspace(workspace), can_process(can_process)
   {

   }

signals:

   void requestProcessSelected();

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

         if (can_process)
            menu.addAction("Process", [this, obj]() { emit requestProcessSelected(); });


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
   bool can_process;
};