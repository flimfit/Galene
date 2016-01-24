
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
   connect(set_output_folder_action, &QAction::triggered, this, &FlimDisplay::setAutoSaveFolder);

   setupTCSPC();
  
   // Setup menu actions
   //============================================================
   //connect(acquire_background_action, &QAction::triggered, processor, &OCTProcessor::AcquireBackground);
   
   connect(scan_button, &QPushButton::toggled, this, &FlimDisplay::setScanning);
   Bind(n_images_spin, this, &FlimDisplay::setNumImages, &FlimDisplay::getNumImages);

   Bind(prefix_edit, workspace, &FlimWorkspace::setFilePrefix, &FlimWorkspace::getFilePrefix);
   Bind(seq_number_spin, workspace, &FlimWorkspace::setSequenceNumber, &FlimWorkspace::getSequenceNumber, &FlimWorkspace::sequenceNumberChanged);

   file_list_view->setModel(workspace);
}

void FlimDisplay::setupTCSPC()
{

      try
      {
         tcspc = new Cronologic(this);
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
   if (tcspc)
      tcspc->setScanning(scanning);
}

void FlimDisplay::acquireSequence()
{
   if (tcspc == nullptr)
      return;

   //QSettings s;
   //QDir path = s.value("sequence_acq_output_folder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
   //QString unique_string = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss");
   //QString flim_file_name = path.filePath(QString("AutoSequence %1 FLIM.spc").arg(unique_string));

   try
   {
      QString flim_file_name = workspace->getNextFileName();


      current_frame = 0;
      auto_sequence_in_progress = true;

      //TODO   connect(scanner, &GalvoScanner::FrameIncremented, this, &FlimDisplay::FrameIncremented);

      setScanning(true);
      tcspc->startRecording(flim_file_name);

      acquire_sequence_button->setEnabled(false);
   }
   catch (std::runtime_error e)
   {
      QMessageBox::critical(this, "Error Occured", e.what());
      return;
   }

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
      tcspc->setRecording(false);
      
      auto_sequence_in_progress = false;
      acquire_sequence_button->setEnabled(true);
   }
   
}

FlimDisplay::~FlimDisplay()
{
   shutdown();
}