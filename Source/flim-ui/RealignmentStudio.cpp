
#include "RealignmentStudio.h"
#include <QFileDialog>
#include <QVector>
#include <QDateTime>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include "Cronologic.h"
#include "ConstrainedMdiSubWindow.h"
#include "FlimFileReader.h"
#include "FlimCubeWriter.h"
#include "ImageRenderWindow.h"
#include "RealignmentImageSource.h"
#include "RealignmentDisplayWidget.h"
#include <functional>
#include <thread>

#define Signal(object, function, type) static_cast<void (object::*)(type)>(&object::function)

using namespace std;

RealignmentStudio::RealignmentStudio() :
   ControlBinder(this, "RealignmentStudio")
{
   setupUi(this);

   workspace = new FlimWorkspace(this);

   connect(export_movie_action, &QAction::triggered, this, &RealignmentStudio::exportMovie);
   connect(workspace, &FlimWorkspace::openRequest, this, &RealignmentStudio::openFile);

   connect(reload_button, &QPushButton::pressed, this, &RealignmentStudio::reload);
   connect(save_button, &QPushButton::pressed, this, &RealignmentStudio::save);

   file_list_view->setModel(workspace);
   file_list_view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
   file_list_view->setEditTriggers(QAbstractItemView::EditKeyPressed);
   file_list_view->installEventFilter(new WorkspaceEventFilter(workspace));
   connect(file_list_view, &QListView::doubleClicked, workspace, &FlimWorkspace::requestOpenFile);
   
   Bind(close_after_save_check, this, &RealignmentStudio::setCloseAfterSave, &RealignmentStudio::getCloseAfterSave);

}

void RealignmentStudio::openFile(const QString& filename) 
{
   try
   {
      auto reader = std::make_shared<FlimReaderDataSource>(filename);
      connect(reader.get(), &FlimReaderDataSource::error, this, &RealignmentStudio::displayErrorMessage);
      reader->getReader()->setRealignmentParameters(getRealignmentParameters());
      std::list<QMdiSubWindow*> windows;

      auto createSubWindow = [&](QWidget* widget) -> QMdiSubWindow*
      {
         auto sub = new ConstrainedMdiSubWindow();
         sub->setWidget(widget);
         sub->setAttribute(Qt::WA_DeleteOnClose);
         mdi_area->addSubWindow(sub);
         
         widget->show();
         widget->setWindowTitle(QFileInfo(filename).baseName());
         window_map[sub] = reader;

         return sub;
      };

      auto widget = new LifetimeDisplayWidget;
      widget->setFlimDataSource(reader);
      auto w1 = createSubWindow(widget);
      
      auto realignment_widget = new RealignmentDisplayWidget(reader);
      auto w2 = createSubWindow(realignment_widget);

      // Make windows close each other
      connect(w1, &QObject::destroyed, w2, &QObject::deleteLater);
      connect(w2, &QObject::destroyed, w1, &QObject::deleteLater);


      reader->readData();
   }
   catch (std::runtime_error e)
   {
      QMessageBox::warning(this, "Error loading file", QString("Could not load file '%1'").arg(filename));
   }
}

std::shared_ptr<FlimReaderDataSource> RealignmentStudio::getCurrentSource()
{
   auto active_window = mdi_area->activeSubWindow();
   auto weak_source = window_map[active_window];

   if (weak_source.expired())
      return nullptr;

   return weak_source.lock();
}

void RealignmentStudio::reload()
{
   auto source = getCurrentSource();
   auto reader = source->getReader();

   reader->setRealignmentParameters(getRealignmentParameters());
   source->readData();
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

void RealignmentStudio::save()
{
   // clean out threads that are finished
   save_thread.remove_if([](std::thread& t) { return !t.joinable(); });

   auto source = getCurrentSource();
   auto window = mdi_area->activeSubWindow();
   QFileInfo fi = QString::fromStdString(source->getReader()->getFilename());
   QString name = fi.baseName() + suffix_edit->text();
   std::string filename = workspace->getFileName(name, ".ffh").toStdString();

   save_thread.push_back(std::thread([this, source, filename, window]()
   {
      auto reader = source->getReader();
      auto data = source->getData();
      auto tags = reader->getTags();
      auto reader_tags = reader->getReaderTags();

      try
      {
         FlimCubeWriter<uint16_t> writer(filename, data, tags, reader_tags);
      }
      catch (std::runtime_error e)
      {
         displayErrorMessage(e.what());
      }

      if (close_after_save)
         QMetaObject::invokeMethod(window, "close");

   }));
   

}

RealignmentParameters RealignmentStudio::getRealignmentParameters()
{
   RealignmentParameters params;

   params.type = static_cast<RealignmentType>(mode_combo->currentIndex());
   params.n_resampling_points = realignment_points_spin->value();
   params.frame_binning = frame_binning_combo->value();
   params.spatial_binning = pow(2, spatial_binning_combo->currentIndex());

   return params;
}


void RealignmentStudio::displayErrorMessage(const QString msg)
{
   QMessageBox::critical(this, "Error", msg);
}

RealignmentStudio::~RealignmentStudio()
{
}