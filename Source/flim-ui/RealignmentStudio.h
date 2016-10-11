#pragma once

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QTimer>
#include "qcustomplot.h"

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

   void setStatusBarMessage(const QString& message) { statusbar->showMessage(message); }
   void updateProgress(double progress);

protected:

   void sendStatusUpdate();
   void openFile(const QString& filename);
   std::shared_ptr<FlimReaderDataSource> getCurrentSource();
   void reload();
   void save();
   RealignmentParameters getRealignmentParameters();

private:

   void displayErrorMessage(const QString error);

   LifetimeDisplayWidget* preview_widget;
   FlimWorkspace* workspace;

   QTimer* status_timer;

   std::map<QMdiSubWindow*, std::weak_ptr<FlimReaderDataSource>> window_map;

   std::list<std::thread> save_thread;
};