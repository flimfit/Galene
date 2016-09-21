
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
#include <functional>

#define Signal(object, function, type) static_cast<void (object::*)(type)>(&object::function)

using namespace std;

RealignmentStudio::RealignmentStudio() :
   ControlBinder(this, "RealignmentStudio")
{
   setupUi(this);

   workspace = new FlimWorkspace(this);

   connect(open_workspace_action, &QAction::triggered, workspace, &FlimWorkspace::open);

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

void RealignmentStudio::reload()
{
   auto active_window = mdi_area->activeSubWindow();
   auto weak_source = window_map[active_window];

   if (weak_source.expired())
      return;

   auto source = weak_source.lock();
   auto reader = source->getReader();

   RealignmentParameters params;

   reader->setRealignmentParameters(params);
}


void RealignmentStudio::displayErrorMessage(const QString msg)
{
   QMessageBox::critical(this, "Error", msg);
}

RealignmentStudio::~RealignmentStudio()
{
}