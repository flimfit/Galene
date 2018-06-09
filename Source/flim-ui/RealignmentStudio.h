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

   std::shared_ptr<RealignableDataSource> openFileWithOptions(const QString& filename, RealignableDataOptions& options);
   std::shared_ptr<RealignableDataSource> openFile(const QString& filename);
   void showFileInfo(const QString& filename);

signals:

   void error(const QString& msg);
   void newDataSource(std::shared_ptr<RealignableDataSource> source);

protected:

   std::shared_ptr<RealignableDataSource> getCurrentSource();
   void realign();
   void reload();
   void copyImage();
   void about();
   void exportBioformatsSeries();

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
   bool use_gpu = true;
   int default_reference = 0;
   int mode = (int) RealignmentType::Warp;
   int realignment_points = 10;
   double smoothing = 2;
   double threshold = 0;
   double coverage_threshold = 0;
   bool store_frames = false;
   int spatial_binning = 0;

   Q_PROPERTY(bool close_after_save MEMBER close_after_save);
   Q_PROPERTY(bool save_preview MEMBER save_preview);
   Q_PROPERTY(bool save_movie MEMBER save_movie);
   Q_PROPERTY(bool save_realignment_info MEMBER save_realignment_info);
   Q_PROPERTY(bool use_gpu MEMBER use_gpu);
   Q_PROPERTY(int default_reference MEMBER default_reference);
   Q_PROPERTY(int mode MEMBER mode);
   Q_PROPERTY(int realignment_points MEMBER realignment_points);
   Q_PROPERTY(double smoothing MEMBER smoothing);
   Q_PROPERTY(double threshold MEMBER threshold);
   Q_PROPERTY(double coverage_threshold MEMBER coverage_threshold);
   Q_PROPERTY(bool store_frames MEMBER store_frames);
   Q_PROPERTY(int spatial_binning MEMBER spatial_binning);


   void displayErrorMessage(const QString& error);

   LifetimeDisplayWidget* preview_widget;
   FlimWorkspace* workspace;

   QItemSelectionModel *workspace_selection;

   QTimer* status_timer;

   std::map<QMdiSubWindow*, std::weak_ptr<RealignableDataSource>> window_map;
   std::list<std::thread> save_thread;

   friend class RealignmentStudioBatchProcessor;
};