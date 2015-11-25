#pragma once

#include <QMainWindow>
#include "qcustomplot.h"

#include "ControlBinder.h"
#include "Oscilloscope.h"
#include "ImageRenderWindow.h"
#include "LifetimeDisplayWidget.h"
#include "ui_FlimDisplay.h"
#include "SimTcspc.h"
#include <memory>



class FlimDisplay : public QMainWindow, private ControlBinder, private Ui::FlimDisplay
{
   Q_OBJECT

public:
   explicit FlimDisplay();
   ~FlimDisplay();

   void shutdown();

   void setStatusBarMessage(const QString& message) { statusbar->showMessage(message); }

   void setScanning(bool scanning);
   void setNumImages(int n_seq_images_) { n_seq_images = n_seq_images_; };
   int getNumImages() { return n_seq_images; }
   
   void setAutoSaveFolder() 
   {
      QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
      folder = QFileDialog::getExistingDirectory(nullptr, "Choose an output folder", folder);

      QSettings s;
      s.setValue("sequence_acq_output_folder", folder);
   }

private:

   void setupTCSPC();
   void acquireSequence();

   void positionUpdated(double position);
   void frameIncremented();

   bool auto_sequence_in_progress = false;
   int current_frame = 0;
   int n_seq_images = 10;

   ImageRenderWindow* flim_display = nullptr;

   QWidget* camera_control = nullptr;
   FifoTcspc* tcspc = nullptr;

   LifetimeDisplayWidget* preview_widget;
};
