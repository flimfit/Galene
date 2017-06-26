
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
#include "FlimCubeWriter.h"
#include "ImageRenderWindow.h"
#include "RealignmentImageSource.h"
#include "RealignmentDisplayWidget.h"
#include "RealignmentResultsWriter.h"
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
   connect(export_alignment_info_action, &QAction::triggered, this, &RealignmentStudio::writeAlignmentInfoCurrent);
   connect(workspace, &FlimWorkspace::openRequest, this, &RealignmentStudio::openFile);
   connect(workspace, &FlimWorkspace::infoRequest, this, &RealignmentStudio::showFileInfo);

   connect(realign_button, &QPushButton::pressed, this, &RealignmentStudio::realign);
   connect(reload_button, &QPushButton::pressed, this, &RealignmentStudio::reload);
   connect(save_button, &QPushButton::pressed, this, &RealignmentStudio::saveCurrent);
   connect(process_selected_button, &QPushButton::pressed, this, &RealignmentStudio::processSelected);
   
   connect(mode_combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RealignmentStudio::updateParameterGroupBox);

   connect(this, &RealignmentStudio::newDataSource, this, &RealignmentStudio::openWindows);

   file_list_view->setModel(workspace);
   file_list_view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
   file_list_view->setEditTriggers(QAbstractItemView::EditKeyPressed);

   auto event_filter = new WorkspaceEventFilter(workspace, true);
   connect(event_filter, &WorkspaceEventFilter::requestProcessSelected, this, &RealignmentStudio::processSelected);

   file_list_view->installEventFilter(event_filter);
   connect(file_list_view, &QListView::doubleClicked, workspace, &FlimWorkspace::requestOpenFile);
   
   workspace_selection = file_list_view->selectionModel();

   Bind(close_after_save_check, this, &RealignmentStudio::setCloseAfterSave, &RealignmentStudio::getCloseAfterSave);
   Bind(save_preview_check, this, &RealignmentStudio::setSavePreview, &RealignmentStudio::getSavePreview);
   Bind(save_realignment_info_check, this, &RealignmentStudio::setSaveRealignmentInfo, &RealignmentStudio::getSaveRealignmentInfo);
   Bind(save_movie_check, this, &RealignmentStudio::setSaveMovie, &RealignmentStudio::getSaveMovie);


}

void RealignmentStudio::updateParameterGroupBox(int index)
{
	int page = 0;
	switch (index)
	{
	case 1:
	case 2:
		page = 1;
		break;
	case 3:
		page = 2; 
		break;
	}
	parameter_stackedwidget->setCurrentIndex(page);
}


std::shared_ptr<FlimReaderDataSource> RealignmentStudio::openFile(const QString& filename)
{
   std::shared_ptr<FlimReaderDataSource> source;

   try
   {
      source = std::make_shared<FlimReaderDataSource>(filename);
      connect(source.get(), &FlimReaderDataSource::error, this, &RealignmentStudio::displayErrorMessage);
      source->getReader()->setRealignmentParameters(getRealignmentParameters());

      emit newDataSource(source);
   }
   catch (std::runtime_error e)
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

   connect(widget, &QObject::destroyed, sub, &QObject::deleteLater);

   return sub;
}

void RealignmentStudio::openWindows(std::shared_ptr<FlimReaderDataSource> source)
{
   auto widget = new LifetimeDisplayWidget;
   widget->setFlimDataSource(source);

   QString filename = QString::fromStdString(source->getReader()->getFilename());
   QString title = QFileInfo(filename).baseName();

   auto w1 = createSubWindow(widget, title);
   window_map[w1] = source;

   auto realignment_widget = new RealignmentDisplayWidget(source);
   auto w2 = createSubWindow(realignment_widget, title);
   window_map[w2] = source;

   FlimReader* reader = source->getReader().get();
   connect(realignment_widget, &RealignmentDisplayWidget::referenceIndexUpdated, [reader](int index) { reader->setReferenceIndex(index); }); // reader isn't a QObject

   // Make windows close each other
   connect(w2, &QObject::destroyed, [&]() {
      auto it = window_map.find(w1);
      if (it != window_map.end())
         window_map.erase(w1);
   });

   connect(w1, &QObject::destroyed, [&]() {
      auto it = window_map.find(w2);
      if (it != window_map.end())
         window_map.erase(w2); 
   });

   connect(w1, &QObject::destroyed, w2, &QObject::deleteLater);
   connect(w2, &QObject::destroyed, w1, &QObject::deleteLater);

   source->readData();
}

std::shared_ptr<FlimReaderDataSource> RealignmentStudio::getCurrentSource()
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
   auto reader = source->getReader();

   reader->setRealignmentParameters(getRealignmentParameters());
   source->readData();
}

void RealignmentStudio::reload()
{
   auto source = getCurrentSource();
   auto reader = source->getReader();

   reader->setRealignmentParameters(getRealignmentParameters());
   source->readData(false);
}


void RealignmentStudio::exportMovie()
{
   auto active_window = mdi_area->activeSubWindow();
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

void RealignmentStudio::writeAlignmentInfo(std::shared_ptr<FlimReaderDataSource> source)
{
   auto reader = source->getReader();
   auto& aligner = reader->getFrameAligner();

   if (aligner == nullptr)
      return;

   QFileInfo file(QString::fromStdString(reader->getFilename()));
   QString new_file = file.dir().absoluteFilePath(file.baseName().append("_realignment.csv"));
   std::string s = new_file.toStdString();

   aligner->writeRealignmentInfo(new_file.toStdString());
}


void RealignmentStudio::saveCurrent()
{
	save(getCurrentSource());
}

void RealignmentStudio::save(std::shared_ptr<FlimReaderDataSource> source, bool force_close)
{
   if (source == nullptr)
      return;

   // clean out threads that are finished
   save_thread.remove_if([](std::thread& t) { return !t.joinable(); });

   QFileInfo fi = QString::fromStdString(source->getReader()->getFilename());
   QString name = fi.baseName() + suffix_edit->text();
   std::string filename = workspace->getFileName(name, ".ffh").toStdString();
   std::string preview_filename = workspace->getFileName(name, ".png").toStdString();
   QString aligned_movie_filename = workspace->getFileName(fi.baseName(), "-aligned-stack.tif");
   QString aligned_preserving_movie_filename = workspace->getFileName(fi.baseName(), "-aligned-int-presv-stack.tif");
   QString unaligned_movie_filename = workspace->getFileName(fi.baseName(), "-unaligned-stack.tif");
   QString coverage_movie_filename = workspace->getFileName(fi.baseName(), "-coverage-stack.tif");

   auto task = std::make_shared<TaskProgress>("Saving...");
   TaskRegister::addTask(task);

   save_thread.push_back(std::thread([=]()
   {
      auto reader = source->getReader();
      auto data = source->getData();
      auto tags = reader->getTags();
      auto reader_tags = reader->getReaderTags();
      auto images = reader->getImageMap();

      if (save_preview)
      {
         const cv::Mat& intensity = source->getIntensity();

         double mn, mx;
         cv::minMaxLoc(intensity, &mn, &mx);

         cv::Scalar mean, std;
         cv::meanStdDev(intensity, mean, std);
         double i_max = mean[0] + 1.96 * std[0]; // 97.5% 

         cv::Mat output;
         intensity.convertTo(output, CV_8U, 255.0 / i_max);

#ifndef SUPPRESS_OPENCV_HIGHGUI
         cv::imwrite(preview_filename, output);
#endif
      }
      
      if (save_realignment_info)
	      writeAlignmentInfo(source);

      if (save_movie)
      {
         auto& results = reader->getRealignmentResults();
         RealignmentResultsWriter::exportAlignedMovie(results, aligned_movie_filename);
         RealignmentResultsWriter::exportAlignedIntensityPreservingMovie(results, aligned_preserving_movie_filename);
         RealignmentResultsWriter::exportUnalignedMovie(results, unaligned_movie_filename);
         RealignmentResultsWriter::exportCoverageMovie(results, coverage_movie_filename);
      }


	  try
	  {
		  FlimCubeWriter<uint16_t> writer(filename, data, tags, reader_tags, images);
	  }
	  catch (std::runtime_error e)
	  {
		  displayErrorMessage(e.what());
	  }


      if (close_after_save || force_close)
         source->requestDelete();

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
   params.frame_binning = frame_binning_combo->value();
   params.spatial_binning = pow(2, spatial_binning_combo->currentIndex());
   params.correlation_threshold = threshold_spin->value();
   params.coverage_threshold = coverage_threshold_spin->value() / 100.;
   params.smoothing = smoothing_spin->value();

   return params;
}


void RealignmentStudio::displayErrorMessage(const QString msg)
{
   QMessageBox::critical(this, "Error", msg);
}

RealignmentStudio::~RealignmentStudio()
{
}