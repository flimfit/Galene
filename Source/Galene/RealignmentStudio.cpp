#include "RealignmentStudio.h"
#include "RealignmentStudioBatchProcessor.h"
#include "MetaDataDialog.h"
#include <QFileDialog>
#include <QVector>
#include <QDateTime>
#include <QMessageBox>
#include <QClipboard>
#include <iostream>
#include <fstream>
#include "ConstrainedMdiSubWindow.h"
#include "ImageRenderWindow.h"
#include "RealignmentDisplayWidget.h"
#include "RealignmentResultsWriter.h"
#include "IntensityDataSource.h"
#include "GpuFrameWarper.h"
#include "BioformatsExporter.h"

#include <functional>
#include <thread>

#define Signal(object, function, type) static_cast<void (object::*)(type)>(&object::function)

using namespace std;


RealignmentStudio::RealignmentStudio() :
   ControlBinder(this, "RealignmentStudio")
{
   setupUi(this);

   QStringList types = {
	   "*.ffd",
	   "*.pt3",
	   "*.ffh",
	   "*.ptu",
	   "*.spc",
	   "*.ome.tif",
	   "*.sdt",
	   "*.tif",
	   "*.lsm",
	   "*.ims",
	   "*.ics",
	   "*.lif"};

   workspace = new FlimWorkspace(this, types);

   connect(help_action, &QAction::triggered, []() {  QDesktopServices::openUrl(QUrl("http://galene.readthedocs.io")); });

   connect(open_workspace_action, &QAction::triggered, workspace, &FlimWorkspace::open);
   connect(export_movie_action, &QAction::triggered, this, &RealignmentStudio::exportMovie);
   connect(save_merged_action, &QAction::triggered, this, &RealignmentStudio::saveMergedImage);
   connect(quit_action, &QAction::triggered, this, &RealignmentStudio::close);
   connect(copy_action, &QAction::triggered, this, &RealignmentStudio::copyImage);
   connect(about_action, &QAction::triggered, this, &RealignmentStudio::about);

   connect(export_alignment_info_action, &QAction::triggered, this, &RealignmentStudio::writeAlignmentInfoCurrent);
   connect(workspace, &FlimWorkspace::openRequest, this, &RealignmentStudio::openFile);
   connect(workspace, &FlimWorkspace::infoRequest, this, &RealignmentStudio::showFileInfo);
   connect(realignment_parameters_widget, &RealignmentParametersWidget::realignRequested, this, &RealignmentStudio::realign);
   connect(realignment_parameters_widget, &RealignmentParametersWidget::reloadRequested, this, &RealignmentStudio::reload);
   connect(save_button, &QPushButton::pressed, this, &RealignmentStudio::saveCurrent);
   connect(process_selected_button, &QPushButton::pressed, this, &RealignmentStudio::processSelectedIndividually);
   connect(process_selected_group_button, &QPushButton::pressed, this, &RealignmentStudio::processSelectedAsGroup);

   connect(this, &RealignmentStudio::error, this, &RealignmentStudio::displayErrorMessage, Qt::QueuedConnection);

   connect(this, &RealignmentStudio::newDataSource, this, &RealignmentStudio::openWindows, Qt::QueuedConnection);

   file_list_view->setModel(workspace);
   file_list_view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
   file_list_view->setEditTriggers(QAbstractItemView::EditKeyPressed);

   auto event_filter = new WorkspaceEventFilter(workspace, true);
   connect(event_filter, &WorkspaceEventFilter::requestProcessSelected, this, &RealignmentStudio::processSelectedIndividually);
   connect(event_filter, &WorkspaceEventFilter::requestProcessSelectedAsGroup, this, &RealignmentStudio::processSelectedAsGroup);

   file_list_view->installEventFilter(event_filter);
   connect(file_list_view, &QListView::doubleClicked, workspace, &FlimWorkspace::requestOpenFile);
   
   workspace_selection = file_list_view->selectionModel();

   BindProperty(close_after_save_check, this, close_after_save);
   BindProperty(save_preview_check, this, save_preview);
   BindProperty(save_realignment_info_check, this, save_realignment_info);
   BindProperty(save_movie_check, this, save_movie);

   auto support_info = GpuFrameWarper::getGpuSupportInformation();
   realignment_parameters_widget->setGpuSupportInfo(support_info);

   spectral_correction_files = new SpectralCorrectionListModel(this);
   spectral_correction_list_view->setModel(spectral_correction_files);

   connect(spectral_correction_add_pushbutton, &QPushButton::clicked, this, &RealignmentStudio::addSpectralCorrectionFile);
   connect(spectral_correction_remove_pushbutton, &QPushButton::clicked, this, &RealignmentStudio::removeSelectedSpectralCorrectionFiles);
   
   connect(spectral_correction_group, &QGroupBox::toggled, spectral_correction_frame, &QFrame::setVisible);
}


std::shared_ptr<RealignableDataSource> RealignmentStudio::openFile(const QString& filename)
{
   RealignableDataOptions options;
   return openFileWithOptions(filename, options);
}


std::shared_ptr<RealignableDataSource> RealignmentStudio::openFileWithOptions(const QString& filename, RealignableDataOptions& options)
{
   std::shared_ptr<RealignableDataSource> source;
   QFileInfo file(filename);
   QString ext = file.completeSuffix();

   try
   {
      QStringList supported_extensions = IntensityReader::supportedExtensions();

      if (BioformatsExporter::supportedExtensions().contains(ext))
      {
         BioformatsExporter* exporter = new BioformatsExporter(file);
         exporter->startThread();
      }
      else
      {
         if (IntensityReader::supportedExtensions().contains(ext))
         {
            auto s = std::make_shared<IntensityDataSource>(filename);
            connect(s.get(), &IntensityDataSource::error, this, &RealignmentStudio::displayErrorMessage);
            source = s;
         }
         else
         {
            auto s = std::make_shared<FlimDataSource>(filename);
            auto spectral_correction = spectral_correction_files->getMatching(filename);
            auto reader = s->getReader();
            reader->setSpectralCorrection(spectral_correction);
            reader->setRetainData(true);

            connect(s.get(), &FlimDataSource::error, this, &RealignmentStudio::displayErrorMessage);
            source = s;
         }

         options.requestFromUserIfRequired(source->aligningReader());

         source->setRealignmentOptions(options);

         emit newDataSource(source);

         auto realignment_parameters = realignment_parameters_widget->getParameters();
         source->setRealignmentParameters(realignment_parameters);
         source->readData();
      }
   }
   catch (std::runtime_error e)
   {
      QMessageBox::warning(nullptr, "Error loading file", QString("Could not load file '%1', %2").arg(filename).arg(e.what()));
   }
   catch (std::exception e)
   {
      QMessageBox::warning(nullptr, "Error loading file", QString("Could not load file '%1', %2").arg(filename).arg(e.what()));
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

void RealignmentStudio::copyImage()
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
      cv::Mat im = w->getMergedImage();

      QImage image = CopyToQImage(im, 0);
      QClipboard *clipboard = QApplication::clipboard();
      clipboard->setImage(image);
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

void RealignmentStudio::realign(const RealignmentParameters& params)
{
   auto source = getCurrentSource();
   
   if (!source)
   {
      QMessageBox::warning(this, "Warning", "Please open a file before reloading");
      return;
   }

   source->setRealignmentParameters(params);
   source->readData();
}

void RealignmentStudio::reload(const RealignmentParameters& params)
{
   auto source = getCurrentSource();

   if (!source)
   {
      QMessageBox::warning(this, "Warning", "Please open a file before reloading");
      return;
   }

   
   source->setRealignmentParameters(params);
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

   bool interpolate = interpolate_missing_check->isChecked();

   save_thread.push_back(std::thread([=]()
   {
      try
	   {
         if (save_realignment_info)
            source->writeRealignmentInfo(name);

         source->saveData(name, interpolate);

         if (save_preview)
            source->savePreview(preview_filename);

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

void RealignmentStudio::processSelected(bool as_group)
{
   QStringList files = workspace->getSelectedFiles(workspace_selection->selectedRows());

   // Don't process lif files    TODO: generalise this
   for (auto f = files.begin(); f < files.end(); f++)
      if ((*f).endsWith(".lif"))
         files.erase(f);

   if (!files.isEmpty())
      new RealignmentStudioBatchProcessor(this, files, as_group); // deletes itself
}

void RealignmentStudio::displayErrorMessage(const QString& msg)
{
   QMessageBox::critical(this, "Error", msg);
}

void RealignmentStudio::about()
{

   QString text = QString("Galene ").append(VERSION).append("<br><br><a href='http://galene.flimfit.org/'>Galene Website</a>")
      .append("<br><br><a href='http://galene.flimfit.org/licence.html'>Third party licence Information</a>");

   QMessageBox::about(this, "About Galene", text);
}

RealignmentStudio::~RealignmentStudio()
{
   for(auto& thread : save_thread)
      if(thread.joinable())
         thread.join();
}

void RealignmentStudio::removeSelectedSpectralCorrectionFiles()
{

   QModelIndexList selection = spectral_correction_list_view->selectionModel()->selectedRows();
   spectral_correction_files->remove(selection);

}

void RealignmentStudio::addSpectralCorrectionFile()
{
   QStringList files = QFileDialog::getOpenFileNames(this, "Choose file names", workspace->getWorkspace(), "tif file (*.tif)");
   spectral_correction_files->add(files);
}
