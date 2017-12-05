
#include "RealignmentStudio.h"
#include "RealignmentStudioBatchProcessor.h"
#include "MetaDataDialog.h"
#include <QFileDialog>
#include <QVector>
#include <QDateTime>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include "ConstrainedMdiSubWindow.h"
#include "FlimFileReader.h"
#include "ImageRenderWindow.h"
#include "RealignmentImageSource.h"
#include "RealignmentDisplayWidget.h"
#include "RealignmentResultsWriter.h"
#include "IntensityDataSource.h"
#include "GpuFrameWarper.h"
#include <functional>
#include <thread>

#define Signal(object, function, type) static_cast<void (object::*)(type)>(&object::function)

using namespace std;


RealignmentStudio::RealignmentStudio() :
   ControlBinder(this, "RealignmentStudio")
{
   setupUi(this);

   workspace = new FlimWorkspace(this);

   connect(help_action, &QAction::triggered, []() {  QDesktopServices::openUrl(QUrl("http://galene.readthedocs.io")); });

   connect(open_workspace_action, &QAction::triggered, workspace, &FlimWorkspace::open);
   connect(export_movie_action, &QAction::triggered, this, &RealignmentStudio::exportMovie);
   connect(save_merged_action, &QAction::triggered, this, &RealignmentStudio::saveMergedImage);
   connect(quit_action, &QAction::triggered, this, &RealignmentStudio::close);

   connect(export_alignment_info_action, &QAction::triggered, this, &RealignmentStudio::writeAlignmentInfoCurrent);
   connect(workspace, &FlimWorkspace::openRequest, this, &RealignmentStudio::openFile);
   connect(workspace, &FlimWorkspace::infoRequest, this, &RealignmentStudio::showFileInfo);
   connect(realign_button, &QPushButton::pressed, this, &RealignmentStudio::realign);
   connect(reload_button, &QPushButton::pressed, this, &RealignmentStudio::reload);
   connect(save_button, &QPushButton::pressed, this, &RealignmentStudio::saveCurrent);
   connect(process_selected_button, &QPushButton::pressed, this, &RealignmentStudio::processSelected);
   
   connect(this, &RealignmentStudio::error, this, &RealignmentStudio::displayErrorMessage, Qt::QueuedConnection);

   connect(mode_combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RealignmentStudio::updateParameterGroupBox);

   connect(this, &RealignmentStudio::newDataSource, this, &RealignmentStudio::openWindows, Qt::QueuedConnection);

   file_list_view->setModel(workspace);
   file_list_view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
   file_list_view->setEditTriggers(QAbstractItemView::EditKeyPressed);

   auto event_filter = new WorkspaceEventFilter(workspace, true);
   connect(event_filter, &WorkspaceEventFilter::requestProcessSelected, this, &RealignmentStudio::processSelected);

   file_list_view->installEventFilter(event_filter);
   connect(file_list_view, &QListView::doubleClicked, workspace, &FlimWorkspace::requestOpenFile);
   
   workspace_selection = file_list_view->selectionModel();

   BindProperty(close_after_save_check, this, close_after_save);
   BindProperty(save_preview_check, this, save_preview);
   BindProperty(save_realignment_info_check, this, save_realignment_info);
   BindProperty(save_movie_check, this, save_movie);
   BindProperty(default_reference_combo, this, default_reference);
   BindProperty(use_gpu_check, this, use_gpu);
   BindProperty(mode_combo, this, mode);
   BindProperty(realignment_points_spin, this, realignment_points);
   BindProperty(smoothing_spin, this, smoothing);
   BindProperty(threshold_spin, this, threshold);
   BindProperty(coverage_threshold_spin, this, coverage_threshold);
   BindProperty(force_nz_spin, this, force_nz);
   BindProperty(store_frames_check, this, store_frames);
   BindProperty(spatial_binning_combo, this, spatial_binning);

   if (GpuFrameWarper::hasSupportedGpu())
      use_gpu_check->setEnabled(true);
   else
      use_gpu_check->setChecked(false);
}

void RealignmentStudio::updateParameterGroupBox(int index)
{
	int page = index;
	parameter_stackedwidget->setCurrentIndex(page);
}


std::shared_ptr<RealignableDataSource> RealignmentStudio::openFile(const QString& filename)
{
   std::shared_ptr<RealignableDataSource> source;

   QFileInfo file(filename);
   auto ext = file.completeSuffix();

   try
   {
      if (ext == "ome.tif" || ext == "lsm" || ext == "ims")
      {
         auto s = std::make_shared<IntensityDataSource>(filename);
         connect(s.get(), &IntensityDataSource::error, this, &RealignmentStudio::displayErrorMessage);
         source = s;
      }
      else
      {
         auto s = std::make_shared<FlimReaderDataSource>(filename);
         connect(s.get(), &FlimReaderDataSource::error, this, &RealignmentStudio::displayErrorMessage);
         if (force_nz > 0)
            s->setForceNumZ(force_nz);
         source = s;
      }

      source->requestChannelsFromUser();

      emit newDataSource(source);
      source->setRealignmentParameters(getRealignmentParameters());
      source->readData();

   }
   catch (std::runtime_error e)
   {
      QMessageBox::warning(this, "Error loading file", QString("Could not load file '%1', %2").arg(filename).arg(e.what()));
   }
   catch (std::exception e)
   {
      QMessageBox::warning(this, "Error loading file", QString("Could not load file '%1', %2").arg(filename).arg(e.what()));
   }


   return source;
}

void RealignmentStudio::showFileInfo(const QString& filename)
{
   try
   {
      auto reader = FlimReader::createReader(filename.toStdString());
      auto widget = new MetaDataDialog(reader->getTags());
      createSubWindow(widget, filename);
   }
   catch (std::runtime_error e)
   {
      QMessageBox::warning(this, "Error loading file", QString("Could not load file '%1', %2").arg(filename).arg(e.what()));
   }
}

QMdiSubWindow* RealignmentStudio::createSubWindow(QWidget* widget, const QString& title)
{
   auto sub = new ConstrainedMdiSubWindow();
   sub->setWidget(widget);
   sub->setAttribute(Qt::WA_DeleteOnClose);
   mdi_area->addSubWindow(sub);

   widget->show();
   widget->setWindowTitle(title);

   connect(sub, &QObject::destroyed, widget, &QObject::deleteLater);
   connect(widget, &QObject::destroyed, sub, &QObject::deleteLater);

   return sub;
}

void RealignmentStudio::closeEvent(QCloseEvent* event)
{
   auto task_reg = TaskRegister::getRegister();
   bool has_tasks = !task_reg->getTasks().empty();

   if (has_tasks)
   {
      QMessageBox::warning(this, "Warning", "Please wait for tasks to complete");
      event->ignore();
      return;
   }

   // Close each subwindow before closing main window
   while (!window_map.empty())
   {
      auto w = window_map.begin()->first;
      window_map.erase(window_map.begin());
      w->close();
   }
   event->accept();
}


void RealignmentStudio::openWindows(std::shared_ptr<RealignableDataSource> source)
{
   QString filename = source->getFilename();
   QString title = QFileInfo(filename).baseName();

   auto realignment_widget = new RealignmentDisplayWidget(source);
   auto w2 = createSubWindow(realignment_widget, title);
   window_map[w2] = source;

   connect(w2, &QObject::destroyed, this, &RealignmentStudio::removeWindow);

   connect(realignment_widget, &RealignmentDisplayWidget::referenceIndexUpdated, [source](int index) { source->setReferenceIndex(index); }); // source isn't a QObject

   QWidget* widget = source->getWidget();
   if (widget)
   {
      auto w1 = createSubWindow(widget, title);
      window_map[w1] = source;
      
      connect(w1, &QObject::destroyed, this, &RealignmentStudio::removeWindow);
      
      // Make windows close each other
      connect(w1, &QObject::destroyed, w2, &QObject::deleteLater);
      connect(w2, &QObject::destroyed, w1, &QObject::deleteLater);   
   }
}

void RealignmentStudio::removeWindow(QObject* obj)
{
   QMdiSubWindow* w = static_cast<QMdiSubWindow*>(obj);
   auto it = window_map.find(w);
   if (it != window_map.end())
      window_map.erase(w);
};

std::shared_ptr<RealignableDataSource> RealignmentStudio::getCurrentSource()
{
   auto active_window = mdi_area->activeSubWindow();
   auto weak_source = window_map[active_window];

   if (weak_source.expired())
      return nullptr;

   return weak_source.lock();
}

void RealignmentStudio::realign()
{
   auto source = getCurrentSource();
   
   if (!source)
   {
      QMessageBox::warning(this, "Warning", "Please open a file before reloading");
      return;
   }

   source->setRealignmentParameters(getRealignmentParameters());
   source->readData();
}

void RealignmentStudio::reload()
{
   auto source = getCurrentSource();

   if (!source)
   {
      QMessageBox::warning(this, "Warning", "Please open a file before reloading");
      return;
   }

   source->setRealignmentParameters(getRealignmentParameters());
   source->readData(false);
}


void RealignmentStudio::exportMovie()
{
   auto active_window = mdi_area->activeSubWindow();

   if (!active_window)
   {
      QMessageBox::warning(this, "Warning", "No FLIM image open");
      return;
   }

   auto main_w = active_window->widget();
   if (main_w->inherits("RealignmentDisplayWidget"))
   {
      auto w = reinterpret_cast<RealignmentDisplayWidget*>(main_w);
      w->exportMovie();
   }

}

void RealignmentStudio::saveMergedImage()
{
   auto active_window = mdi_area->activeSubWindow();

   if (!active_window)
   {
     QMessageBox::warning(this, "Warning", "No FLIM image open");
     return;
   }

   auto main_w = active_window->widget();
   if (main_w->inherits("LifetimeDisplayWidget"))
   {
      auto w = reinterpret_cast<LifetimeDisplayWidget*>(main_w);
      w->saveMergedImage();
   }

}


void RealignmentStudio::writeAlignmentInfoCurrent()
{
	writeAlignmentInfo(getCurrentSource());
}

void RealignmentStudio::writeAlignmentInfo(std::shared_ptr<RealignableDataSource> source)
{
   if (source)
   {
      QString fileroot = QFileInfo(source->getFilename()).baseName();
      source->writeRealignmentInfo(fileroot);      
   }
   else
      QMessageBox::warning(this, "Warning", "No FLIM image open");
}


void RealignmentStudio::saveCurrent()
{
	save(getCurrentSource());
}

void RealignmentStudio::save(std::shared_ptr<RealignableDataSource> source, bool force_close)
{
   if (source == nullptr)
      return;

   // clean out threads that are finished
   save_thread.remove_if([](std::thread& t) { return !t.joinable(); });

   QFileInfo fi = source->getFilename();
   QString name = fi.absoluteDir().filePath(fi.baseName() + suffix_edit->text());
   QString preview_filename = name + ".png";

   auto task = std::make_shared<TaskProgress>("Saving...");
   TaskRegister::addTask(task);

   save_thread.push_back(std::thread([=]()
   {
      try
	   {
         source->saveData(name);

         if (save_preview)
            source->savePreview(preview_filename);

         if (save_realignment_info)
            source->writeRealignmentInfo(name);

         if (save_movie)
            source->writeRealignmentMovies(name);
	   }
	   catch (std::runtime_error e)
	   {
		   emit error(e.what());
	   }

      if (close_after_save || force_close)
      {
         for (auto& w : window_map)
            if (w.second.lock() == source)
               QMetaObject::invokeMethod(w.first, "close", Qt::QueuedConnection);
      }

      task->setFinished();
   }));
   

}

void RealignmentStudio::processSelected()
{
   QStringList files = workspace->getSelectedFiles(workspace_selection->selectedRows());
   new RealignmentStudioBatchProcessor(this, files); // deletes itself
}

RealignmentParameters RealignmentStudio::getRealignmentParameters()
{
   RealignmentParameters params;



   params.type = static_cast<RealignmentType>(mode_combo->currentIndex());
   params.n_resampling_points = realignment_points_spin->value();
   
   switch (params.type)
   {
   case RealignmentType::Warp:
      params.frame_binning = 1;
      params.spatial_binning = 1;
      break;
   default:
      params.frame_binning = frame_binning_combo->value();
      params.spatial_binning = pow(2, spatial_binning_combo->currentIndex());
   }

   params.spatial_binning = pow(2, spatial_binning_combo->currentIndex());


   params.correlation_threshold = threshold_spin->value();
   params.coverage_threshold = coverage_threshold_spin->value() / 100.;
   params.smoothing = smoothing_spin->value();
   params.prefer_gpu = use_gpu_check->isChecked();
   params.default_reference_frame = (DefaultReferenceFrame) default_reference_combo->currentIndex();
   params.store_frames = store_frames_check->isChecked();

   return params;
}


void RealignmentStudio::displayErrorMessage(const QString& msg)
{
   QMessageBox::critical(this, "Error", msg);
}

RealignmentStudio::~RealignmentStudio()
{
   for(auto& thread : save_thread)
      if(thread.joinable())
         thread.join();
}