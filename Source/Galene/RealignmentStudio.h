#pragma once

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QTimer>
#include "qcustomplot.h"

#include "ThreadedObject.h"
#include "ControlBinder.h"
#include "LifetimeDisplayWidget.h"
#include "FlimWorkspace.h"
#include "FlimDataSource.h"
#include "SpectralCorrectionListModel.h"

#include "ui_RealignmentStudio.h"
#include <map>
#include <list>
#include <vector>

   
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
   void realign(const RealignmentParameters& params);
   void reload(const RealignmentParameters& params);
   void copyImage();
   void about();
   void exportBioformatsSeries();

   void openWindows(std::shared_ptr<RealignableDataSource> source);
   void removeWindow(QObject* obj);

   void saveCurrent();
   void save(std::shared_ptr<RealignableDataSource> source, bool force_close = false);

   void writeAlignmentInfoCurrent();
   void writeAlignmentInfo(std::shared_ptr<RealignableDataSource> source);

   QMdiSubWindow* createSubWindow(QWidget* widget, const QString& title);

   void processSelectedIndividually() { processSelected(false); }
   void processSelectedAsGroup() { processSelected(true); }
   void processSelected(bool as_group);

   void exportMovie();
   void saveMergedImage();

   void removeSelectedSpectralCorrectionFiles();
   void requestSpectralCorrectionFile();
   void addSpectralCorrectionFile(const QString& file);
   void addSpectralCorrectionFiles(const QStringList& file);

private:

   bool close_after_save = false;
   bool save_preview = false;
   bool save_movie = false;
   bool save_realignment_info = false;

   Q_PROPERTY(bool close_after_save MEMBER close_after_save);
   Q_PROPERTY(bool save_preview MEMBER save_preview);
   Q_PROPERTY(bool save_movie MEMBER save_movie);
   Q_PROPERTY(bool save_realignment_info MEMBER save_realignment_info);

   SpectralCorrectionListModel* spectral_correction_files;
   QMenu* recent_spectral_correction_files_menu;

   void displayErrorMessage(const QString& error);

   LifetimeDisplayWidget* preview_widget;
   FlimWorkspace* workspace;

   QItemSelectionModel *workspace_selection;

   QTimer* status_timer;

   std::map<QMdiSubWindow*, std::weak_ptr<RealignableDataSource>> window_map;
   std::list<std::thread> save_thread;

   friend class RealignmentStudioBatchProcessor;
};