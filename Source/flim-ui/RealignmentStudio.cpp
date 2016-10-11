
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
#include <functional>
#include <thread>

#define Signal(object, function, type) static_cast<void (object::*)(type)>(&object::function)

using namespace std;

RealignmentStudio::RealignmentStudio() :
   ControlBinder(this, "RealignmentStudio")
{
   setupUi(this);

   workspace = new FlimWorkspace(this);

   connect(open_workspace_action, &QAction::triggered, workspace, &FlimWorkspace::open);

   connect(reload_button, &QPushButton::pressed, this, &RealignmentStudio::reload);
   connect(save_button, &QPushButton::pressed, this, &RealignmentStudio::save);

   // Setup menu actions
   //============================================================
   file_list_view->setModel(workspace);
   file_list_view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
   file_list_view->setEditTriggers(QAbstractItemView::EditKeyPressed);
   file_list_view->installEventFilter(new WorkspaceEventFilter(workspace));
   connect(file_list_view, &QListView::doubleClicked, workspace, &FlimWorkspace::requestOpenFile);

   connect(workspace, &FlimWorkspace::openRequest, this, &RealignmentStudio::openFile);

}

void RealignmentStudio::openFile(const QString& filename) 
{
   try
   {
      auto reader = std::make_shared<FlimReaderDataSource>(filename);
      connect(reader.get(), &FlimReaderDataSource::error, this, &RealignmentStudio::displayErrorMessage);
      reader->getReader()->setRealignmentParameters(getRealignmentParameters());

      LifetimeDisplayWidget* widget = new LifetimeDisplayWidget;
      ConstrainedMdiSubWindow* sub = new ConstrainedMdiSubWindow();
      sub->setWidget(widget);
      sub->setAttribute(Qt::WA_DeleteOnClose);
      mdi_area->addSubWindow(sub);
      widget->setFlimDataSource(reader);
      widget->show();
      widget->setWindowTitle(QFileInfo(filename).baseName());

      window_map[sub] = reader;
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

void RealignmentStudio::save()
{
   // clean out threads that are finished
   save_thread.remove_if([](std::thread& t) { return !t.joinable(); });

   auto source = getCurrentSource();
   QFileInfo fi = QString::fromStdString(source->getReader()->getFilename());
   QString name = fi.baseName() + suffix_edit->text();
   std::string filename = workspace->getFileName(name, ".ffh").toStdString();

   save_thread.push_back(std::thread([this, source, filename]()
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