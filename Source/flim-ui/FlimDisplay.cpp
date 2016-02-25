
#include "FlimDisplay.h"
#include <QFileDialog>
#include <QVector>
#include <QDateTime>
#include <iostream>
#include <fstream>
#include "Cronologic.h"
#include "ConstrainedMdiSubWindow.h"
#include "FifoTcspcControlDisplayFactory.h"

#define Signal(object, function, type) static_cast<void (object::*)(type)>(&object::function)

using namespace std;

FlimDisplay::FlimDisplay() :
ControlBinder(this, "FLIMDisplay")
{
   setupUi(this);

   workspace = new FlimWorkspace(this);

   connect(new_workspace_action, &QAction::triggered, workspace, &FlimWorkspace::makeNew);
   connect(open_workspace_action, &QAction::triggered, workspace, &FlimWorkspace::open);

   connect(acquire_sequence_button, &QPushButton::pressed, this, &FlimDisplay::acquireSequence);
   connect(stop_button, &QPushButton::pressed, this, &FlimDisplay::stopSequence);

   file_writer = std::make_shared<FlimFileWriter>();


   setupTCSPC();
  
   // Setup menu actions
   //============================================================
   connect(tcspc_settings_action, &QAction::triggered, this, &FlimDisplay::showTcspcSettings);

   connect(live_button, &QPushButton::toggled, this, &FlimDisplay::setLive);
   
   Bind(prefix_edit, workspace, &FlimWorkspace::setFilePrefix, &FlimWorkspace::getFilePrefix);
   Bind(seq_number_spin, workspace, &FlimWorkspace::setSequenceNumber, &FlimWorkspace::getSequenceNumber, &FlimWorkspace::sequenceNumberChanged);

   file_list_view->setModel(workspace);
}

void FlimDisplay::setupTCSPC()
{
   bool use_simulated = false;
   try
   {
      if (use_simulated)
      {
         tcspc = new SimTcspc(this); //Cronologic(this);
         tcspc_control = new QWidget();
      }
      else
      {
         Cronologic* c = new Cronologic(this);
         tcspc_control = new CronologicControlDisplay(c);
         tcspc = c;
      }

   }
   catch (std::runtime_error e)
   {
      QMessageBox msg(QMessageBox::Critical, "Critial Error", QString("Could not connect to TCSPC card: %1").arg(e.what()));
      msg.exec();
      return;
   }

   file_writer->setFifoTcspc(tcspc);

   tcspc_control->setWindowTitle("TCSPC Settings");

   connect(tcspc, &FifoTcspc::acquisitionStatusChanged, this, &FlimDisplay::acquisitionStatusChanged);
   connect(tcspc, &FifoTcspc::ratesUpdated, bh_rates_widget, &BHRatesWidget::SetRates, Qt::QueuedConnection);
   connect(tcspc, &FifoTcspc::fifoUsageUpdated, bh_rates_widget, &BHRatesWidget::SetFifoUsage);
   connect(tcspc, &FifoTcspc::progressUpdated, this, &FlimDisplay::updateProgress);

   tcspc->addTcspcEventConsumer(file_writer);
   Bind(frame_accumulation_spin, tcspc, &FifoTcspc::setFrameAccumulation, &FifoTcspc::getFrameAccumulation);
   Bind(n_images_spin, tcspc, &FifoTcspc::setNumImages, &FifoTcspc::getNumImages);

   //flim_display = new ImageRenderWindow(nullptr, "FLIM", tcspc);
   //flim_display->setWindowTitle("Preview");
   //mdi_area->addSubWindow(flim_display);

   preview_widget = new LifetimeDisplayWidget;
   ConstrainedMdiSubWindow* sub = new ConstrainedMdiSubWindow();
   sub->setWidget(preview_widget);
   sub->setAttribute(Qt::WA_DeleteOnClose);
   mdi_area->addSubWindow(sub);

   preview_widget->setFLIMage(tcspc->getPreviewFLIMage());
}

void FlimDisplay::showTcspcSettings()
{
   if (tcspc_settings_window == nullptr)
   {
      tcspc_settings_window = mdi_area->addSubWindow(tcspc_control);
      tcspc_settings_window->show();
      connect(tcspc_settings_window, &QObject::destroyed, [&]() {
         tcspc_control->setParent(this);
         tcspc_settings_window = nullptr;
      });
   }
}

void EmptyLayout(QLayout* layout)
{
   if (layout->isEmpty())
      return;

   QLayoutItem *child;
   while ((child = layout->takeAt(0)) != 0)
   {
      if (child->widget())
         delete child->widget();
      if (child->layout())
      {
         EmptyLayout(child->layout());
         delete child->layout();
      }
   }
   delete child;

}


void FlimDisplay::shutdown()
{
//   if (tcspc)
//      tcspc->setScanning(false);
}


void FlimDisplay::updateProgress(double progress)
{
   progress_bar->setValue(progress * 100);
}


void FlimDisplay::setLive(bool scanning)
{
   acquire_sequence_button->setEnabled(!scanning);
   if (tcspc)
      tcspc->setLive(scanning);
}

void FlimDisplay::acquisitionStatusChanged(bool acq_in_progress)
{
   acquire_sequence_button->setEnabled(!acq_in_progress);
   live_button->setEnabled(!acq_in_progress);
   stop_button->setEnabled(acq_in_progress);
   progress_bar->setEnabled(acq_in_progress);

   if (!acq_in_progress)
      progress_bar->setValue(0);
}

void FlimDisplay::acquireSequence()
{
   if (tcspc == nullptr)
      return;

   try
   {
      QString flim_file_name = workspace->getNextFileName();
      file_writer->startRecording(flim_file_name);

      tcspc->startAcquisition();
   }
   catch (std::runtime_error e)
   {
      stopSequence();
      QMessageBox::critical(this, "Error Occured", e.what());
      return;
   }

}

void FlimDisplay::stopSequence()
{
   file_writer->stopRecording();
   tcspc->cancelAcquisition();
}

/*
void FlimDisplay::frameIncremented()
{
   if (!auto_sequence_in_progress)
      return;

   current_frame++;

   if (current_frame == n_seq_images)
   {
      QApplication::beep();
      QApplication::beep();

      setScanning(false);
      
      auto_sequence_in_progress = false;
      acquire_sequence_button->setEnabled(true);
   }
   
}
*/

FlimDisplay::~FlimDisplay()
{
   shutdown();
}