#pragma once

#include <QMainWindow>
#include "qcustomplot.h"

#include "ControlBinder.h"
#include "Oscilloscope.h"
#include "ImageRenderWindow.h"
#include "LifetimeDisplayWidget.h"
#include "SimTcspc.h"
#include "FlimFileWriter.h"
#include "FlimWorkspace.h"

#include "ui_FlimDisplay.h"
#include <memory>




class FlimDisplay : public QMainWindow, private ControlBinder, private Ui::FlimDisplay
{
   Q_OBJECT

public:
   explicit FlimDisplay();
   ~FlimDisplay();

   void shutdown();

   void setStatusBarMessage(const QString& message) { statusbar->showMessage(message); }

   void setLive(bool live);

   void updateProgress(double progress);

private:

   void setupTCSPC();
   void acquireSequence();
   void stopSequence();

   void acquisitionStatusChanged(bool acq_in_progress);

   ImageRenderWindow* flim_display = nullptr;

   QWidget* camera_control = nullptr;
   FifoTcspc* tcspc = nullptr;

   std::shared_ptr<FlimFileWriter> file_writer;
   LifetimeDisplayWidget* preview_widget;
   FlimWorkspace* workspace;
};
