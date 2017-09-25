#pragma once

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QTimer>
#include "qcustomplot.h"

#include "ThreadedObject.h"
#include "ControlBinder.h"
#include "LifetimeDisplayWidget.h"
#include "FlimWorkspace.h"
#include "FlimReaderDataSource.h"

#include "ui_RealignmentStudio.h"
#include <map>
#include <list>

class RealignmentStudio : public QMainWindow, private ControlBinder, private Ui::RealignmentStudio
{
   Q_OBJECT

public:
   explicit RealignmentStudio();
   ~RealignmentStudio();

   void closeEvent(QCloseEvent* event);

   void setCloseAfterSave(bool close_after_save_) { close_after_save = close_after_save_; }
   bool getCloseAfterSave() { return close_after_save; }

   void setSavePreview(bool save_preview_) { save_preview = save_preview_; };
   bool getSavePreview() { return save_preview; }

   void setSaveMovie(bool save_movie_) { save_movie = save_movie_; };
   bool getSaveMovie() { return save_movie; }

   void setSaveRealignmentInfo(bool save_realignment_info_) { save_realignment_info = save_realignment_info_; };
   bool getSaveRealignmentInfo() { return save_realignment_info; }

   std::shared_ptr<RealignableDataSource> openFile(const QString& filename);
   void showFileInfo(const QString& filename);

signals:

   void newDataSource(std::shared_ptr<RealignableDataSource> source);

protected:

   std::shared_ptr<RealignableDataSource> getCurrentSource();
   void realign();
   void reload();

   void updateParameterGroupBox(int index);

   void openWindows(std::shared_ptr<RealignableDataSource> source);
   void removeWindow(QObject* obj);

   void saveCurrent();
   void save(std::shared_ptr<RealignableDataSource> source, bool force_close = false);

   void writeAlignmentInfoCurrent();
   void writeAlignmentInfo(std::shared_ptr<RealignableDataSource> source);

   QMdiSubWindow* createSubWindow(QWidget* widget, const QString& title);

   void processSelected();
   void exportMovie();
   void saveMergedImage();

   RealignmentParameters getRealignmentParameters();

private:

   bool close_after_save = false;
   bool save_preview = false;
   bool save_movie = false;
   bool save_realignment_info = false;

   void displayErrorMessage(const QString error);

   LifetimeDisplayWidget* preview_widget;
   FlimWorkspace* workspace;

   QItemSelectionModel *workspace_selection;

   QTimer* status_timer;

   std::map<QMdiSubWindow*, std::weak_ptr<RealignableDataSource>> window_map;
   std::list<std::thread> save_thread;

   friend class RealignmentStudioBatchProcessor;
};