
#include "FlimDisplay.h"
#include <QFileDialog>
#include <QVector>
#include <QDateTime>
#include <iostream>
#include <fstream>
#include "Cronologic.h"
#include "ConstrainedMdiSubWindow.h"

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
   connect(set_output_folder_action, &QAction::triggered, this, &FlimDisplay::setAutoSaveFolder);

   file_writer = std::make_shared<FlimFileWriter>();


   setupTCSPC();
  
   // Setup menu actions
   //============================================================
   //connect(acquire_background_action, &QAction::triggered, processor, &OCTProcessor::AcquireBackground);
   
   connect(live_button, &QPushButton::toggled, this, &FlimDisplay::setScanning);
   Bind(n_images_spin, this, &FlimDisplay::setNumImages, &FlimDisplay::getNumImages);

   Bind(prefix_edit, workspace, &FlimWorkspace::setFilePrefix, &FlimWorkspace::getFilePrefix);
   Bind(seq_number_spin, workspace, &FlimWorkspace::setSequenceNumber, &FlimWorkspace::getSequenceNumber, &FlimWorkspace::sequenceNumberChanged);

   file_list_view->setModel(workspace);
}

void FlimDisplay::setupTCSPC()
{

   try
   {
      tcspc = new SimTcspc(this);
   }
   catch (std::runtime_error e)
   {
      QMessageBox msg(QMessageBox::Critical, "Critial Error", QString("Could not connect to TCSPC card: %1").arg(e.what()));
      msg.exec();
      return;
   }

   file_writer->setFifoTcspc(tcspc);

   connect(tcspc, &FifoTcspc::ratesUpdated, bh_rates_widget, &BHRatesWidget::SetRates, Qt::QueuedConnection);
   connect(tcspc, &FifoTcspc::fifoUsageUpdated, bh_rates_widget, &BHRatesWidget::SetFifoUsage);
   tcspc->addTcspcEventConsumer(file_writer);
   Bind(frame_accumulation_spin, tcspc, &FifoTcspc::setFrameAccumulation, &FifoTcspc::getFrameAccumulation);

   //flim_display = new ImageRenderWindow(nullptr, "FLIM", tcspc);
   //flim_display->setWindowTitle("Preview");
   //mdi_area->addSubWindow(flim_display);

   preview_widget = new LifetimeDisplayWidget;
   ConstrainedMdiSubWindow* sub = new ConstrainedMdiSubWindow();
   sub->setWidget(preview_widget);
   mdi_area->addSubWindow(sub);

   preview_widget->setFLIMage(tcspc->getPreviewFLIMage());
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
   if (tcspc)
      tcspc->setScanning(false);
}



void FlimDisplay::setScanning(bool scanning)
{
   acquire_sequence_button->setEnabled(!scanning);
   if (tcspc)
      tcspc->setScanning(scanning);
}

void FlimDisplay::acquireSequence()
{
   if (tcspc == nullptr)
      return;

   try
   {
      current_frame = 0;
      auto_sequence_in_progress = true;

      QString flim_file_name = workspace->getNextFileName();
      file_writer->startRecording(flim_file_name);

      setScanning(true);

      live_button->setEnabled(false);
      stop_button->setEnabled(true);
   }
   catch (std::runtime_error e)
   {
      QMessageBox::critical(this, "Error Occured", e.what());
      return;
   }

}

void FlimDisplay::stopSequence()
{

   file_writer->stopRecording();
   setScanning(false);

   stop_button->setEnabled(false);
   live_button->setEnabled(true);

}

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

FlimDisplay::~FlimDisplay()
{
   shutdown();
}