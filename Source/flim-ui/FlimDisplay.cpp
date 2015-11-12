
#include "FlimDisplay.h"
#include <QFileDialog>
#include <QVector>
#include <QDateTime>
#include <iostream>
#include <fstream>

#define Signal(object, function, type) static_cast<void (object::*)(type)>(&object::function)

using namespace std;

FlimDisplay::FlimDisplay() :
ControlBinder(this, "HandheldScanner")
{
   setupUi(this);

   connect(acquire_sequence_button, &QPushButton::pressed, this, &FlimDisplay::AcquireSequence);
   connect(set_output_folder_action, &QAction::triggered, this, &FlimDisplay::SetAutoSaveFolder);

   SetupBH();
  
   // Setup menu actions
   //============================================================
   //connect(acquire_background_action, &QAction::triggered, processor, &OCTProcessor::AcquireBackground);
   
   Bind(n_auto_frames_spin, this, &FlimDisplay::SetNumAutoFrames, &FlimDisplay::GetNumAutoFrames);
   connect(scan_button, &QPushButton::toggled, this, &FlimDisplay::SetScanning);


}

void FlimDisplay::SetupBH()
{
   try
   {
      bh = new BH(this, M_SPC830);
      
      bh->SetImageSize(scanner->GetNLines());      
      connect(scanner, &GalvoScanner::ScanSizeChanged, bh, &BH::SetImageSize);
      connect(bh, &BH::RatesUpdated, bh_rates_widget, &BHRatesWidget::SetRates);
      connect(bh, &BH::FifoUsageUpdated, bh_rates_widget, &BHRatesWidget::SetFifoUsage);
      connect(bh, &BH::RecordingStatusChanged, save_flim_action, &QAction::setChecked);
      connect(save_flim_action, &QAction::toggled, bh, &BH::SetRecording, Qt::DirectConnection);


      Bind(frame_accumulation_spin, bh, &BH::SetFrameAccumulation, &BH::GetFrameAccumulation);

      flim_display = new ImageRenderWindow("FLIM", bh);
      mdi_area->addSubWindow(flim_display);
   }
   catch (std::exception e)
   {
      std::cout << e.what() << "\n";
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


void FlimDisplay::Shutdown()
{
   if (bh)
      bh->SetScanning(false);
}



void FlimDisplay::SetScanning(bool scanning)
{
   if (bh)
      bh->SetScanning(scanning);
}

void FlimDisplay::AcquireSequence()
{
   if (bh == nullptr)
      return;

   QSettings s;
   QDir path = s.value("sequence_acq_output_folder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();

   QString unique_string = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss");

   QString flim_file_name = path.filePath(QString("AutoSequence %1 FLIM.spc").arg(unique_string));
   QString tracking_file_name = path.filePath(QString("AutoSequence %1 tracking.csv").arg(unique_string));

   current_frame = 0;
   auto_sequence_in_progress = true;

   connect(scanner, &GalvoScanner::FrameIncremented, this, &FlimDisplay::FrameIncremented);

   SetScanning(true);
   bh->StartRecording(flim_file_name);

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

      controller_mode_combo->setCurrentIndex(0); //DriveMode::Stationary - should do this with a callback...
   }
   else if (current_frame == n_auto_frames * 2)
   {
      QApplication::beep();
      QApplication::beep();

      SetScanning(false);
      bh->SetRecording(false);
      
      auto_sequence_in_progress = false;
      acquire_sequence_button->setEnabled(true);
   }
   
}

FlimDisplay::~FlimDisplay()
{
   Shutdown();
}