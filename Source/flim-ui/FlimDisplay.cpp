
#include "FlimDisplay.h"
#include <QFileDialog>
#include <QVector>
#include <QDateTime>
#include <iostream>
#include <fstream>
#include "SimTcspc.h"

#define Signal(object, function, type) static_cast<void (object::*)(type)>(&object::function)

using namespace std;

FlimDisplay::FlimDisplay() :
ControlBinder(this, "HandheldScanner")
{
   setupUi(this);

   connect(acquire_sequence_button, &QPushButton::pressed, this, &FlimDisplay::AcquireSequence);
   connect(set_output_folder_action, &QAction::triggered, this, &FlimDisplay::SetAutoSaveFolder);

   SetupTCSPC();
  
   // Setup menu actions
   //============================================================
   //connect(acquire_background_action, &QAction::triggered, processor, &OCTProcessor::AcquireBackground);
   
   Bind(n_auto_frames_spin, this, &FlimDisplay::SetNumAutoFrames, &FlimDisplay::GetNumAutoFrames);
   connect(scan_button, &QPushButton::toggled, this, &FlimDisplay::SetScanning);

}

void FlimDisplay::SetupTCSPC()
{

      try
      {
         int a = 0;
         tcspc = new SimTcspc(this);
      }
      catch (std::runtime_error e)
      {
         QMessageBox msg(QMessageBox::Critical, "Critial Error", QString("Could not connect to TCSPC card: %1").arg(e.what()));
         msg.exec();
         return;
      }
 
      connect(tcspc, &FifoTcspc::ratesUpdated, bh_rates_widget, &BHRatesWidget::SetRates);
      connect(tcspc, &FifoTcspc::fifoUsageUpdated, bh_rates_widget, &BHRatesWidget::SetFifoUsage);
      connect(tcspc, &FifoTcspc::recordingStatusChanged, save_flim_action, &QAction::setChecked);
      connect(save_flim_action, &QAction::toggled, tcspc, &FifoTcspc::setRecording, Qt::DirectConnection);


      Bind(frame_accumulation_spin, tcspc, &FifoTcspc::setFrameAccumulation, &FifoTcspc::getFrameAccumulation);

      flim_display = new ImageRenderWindow(nullptr, "FLIM", tcspc);
      flim_display->setWindowTitle("Preview");
      mdi_area->addSubWindow(flim_display);

      preview_widget = new LifetimeDisplayWidget(this);
      mdi_area->addSubWindow(preview_widget);

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


void FlimDisplay::Shutdown()
{
   if (tcspc)
      tcspc->setScanning(false);
}



void FlimDisplay::SetScanning(bool scanning)
{
   if (tcspc)
      tcspc->setScanning(scanning);
}

void FlimDisplay::AcquireSequence()
{
   if (tcspc == nullptr)
      return;

   QSettings s;
   QDir path = s.value("sequence_acq_output_folder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();

   QString unique_string = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss");

   QString flim_file_name = path.filePath(QString("AutoSequence %1 FLIM.spc").arg(unique_string));
   QString tracking_file_name = path.filePath(QString("AutoSequence %1 tracking.csv").arg(unique_string));

   current_frame = 0;
   auto_sequence_in_progress = true;

//TODO   connect(scanner, &GalvoScanner::FrameIncremented, this, &FlimDisplay::FrameIncremented);

   SetScanning(true);
   tcspc->startRecording(flim_file_name);

   acquire_sequence_button->setEnabled(false);
}

void FlimDisplay::FrameIncremented()
{
   if (!auto_sequence_in_progress)
      return;

   current_frame++;

   if (current_frame == n_auto_frames)
   {
      QApplication::beep();
   }
   else if (current_frame == n_auto_frames * 2)
   {
      QApplication::beep();
      QApplication::beep();

      SetScanning(false);
      tcspc->setRecording(false);
      
      auto_sequence_in_progress = false;
      acquire_sequence_button->setEnabled(true);
   }
   
}

FlimDisplay::~FlimDisplay()
{
   Shutdown();
}