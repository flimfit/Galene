#pragma once

#include <QMainWindow>
#include "qcustomplot.h"

#include "ControlBinder.h"
#include "Oscilloscope.h"
#include "ImageRenderWindow.h"
#include "ui_FlimDisplay.h"
#include <memory>



class FlimDisplay : public QMainWindow, private ControlBinder, private Ui::FlimDisplay
{
   Q_OBJECT

public:
   explicit FlimDisplay();
   ~FlimDisplay();

   void Shutdown();

   void SetStatusBarMessage(const QString& message) { statusbar->showMessage(message); }

   void SetScanning(bool scanning);
   void SetNumAutoFrames(int n_auto_frames_) { n_auto_frames = n_auto_frames_; };
   int GetNumAutoFrames() { return n_auto_frames; }
   
   void SetAutoSaveFolder() 
   {
      QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
      folder = QFileDialog::getExistingDirectory(nullptr, "Choose an output folder", folder);

      QSettings s;
      s.setValue("sequence_acq_output_folder", folder);
   }

private:

   void SetupBH();
   void AcquireSequence();

   void PositionUpdated(double position);
   void FrameIncremented();

   bool auto_sequence_in_progress = false;
   int current_frame = 0;
   int n_auto_frames = 10;

   ImageRenderWindow* flim_display = nullptr;

   QWidget* camera_control = nullptr;
   BH* bh = nullptr;
};
