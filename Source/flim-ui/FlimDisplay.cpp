
#include "FlimDisplay.h"
#include <QFileDialog>
#include <QVector>
#include <QDateTime>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include "Cronologic.h"
#include "ConstrainedMdiSubWindow.h"
#include "FifoTcspcControlDisplayFactory.h"
#include "FlimFileReader.h"

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
   connect(status_timer, &QTimer::timeout, this, &FlimDisplay::sendStatusUpdate);

   file_writer = std::make_shared<FlimFileWriter>();

   server = new FlimServer(this);
   status_timer = new QTimer(this);

   connect(server, &FlimServer::measurementRequest, this, &FlimDisplay::processMeasurementRequest);
   connect(server, &FlimServer::clientError, this, &FlimDisplay::processClientError);
   connect(server, &FlimServer::userBreakRequest, this, &FlimDisplay::processUserBreakRequest);
   connect(this, &FlimDisplay::statusUpdate, server, &FlimServer::sendProgress);


   setupTCSPC();
  
   // Setup menu actions
   //============================================================
   connect(tcspc_settings_action, &QAction::triggered, this, &FlimDisplay::showTcspcSettings);

   connect(live_button, &QPushButton::toggled, this, &FlimDisplay::setLive);
   
   Bind(prefix_edit, workspace, &FlimWorkspace::setFilePrefix, &FlimWorkspace::getFilePrefix);
   Bind(seq_number_spin, workspace, &FlimWorkspace::setSequenceNumber, &FlimWorkspace::getSequenceNumber, &FlimWorkspace::sequenceNumberChanged);

   file_list_view->setModel(workspace);
   file_list_view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
   file_list_view->setEditTriggers(QAbstractItemView::EditKeyPressed);
   file_list_view->installEventFilter(new WorkspaceEventFilter(workspace));
   connect(file_list_view, &QListView::doubleClicked, workspace, &FlimWorkspace::requestOpenFile);

   connect(workspace, &FlimWorkspace::openRequest, [&](const QString& filename) {
      try
      {

         FlimFileReader* reader = new FlimFileReader(filename);

         LifetimeDisplayWidget* widget = new LifetimeDisplayWidget;
         ConstrainedMdiSubWindow* sub = new ConstrainedMdiSubWindow();
         sub->setWidget(widget);
         sub->setAttribute(Qt::WA_DeleteOnClose);
         mdi_area->addSubWindow(sub);
         widget->setFLIMage(reader->getFLIMage());
         widget->show();
         widget->setWindowTitle(QFileInfo(filename).baseName());
      }
      catch (std::runtime_error e)
      {
         QMessageBox::warning(this, "Error loading file", QString("Could not load file '%1'").arg(filename));
      }
   });

}

void FlimDisplay::processMeasurementRequest(T_DATAFRAME_SRVREQUEST request, std::map<QString, QVariant> metadata)
{
   if (!tcspc->readyForAcquisition())
   {
      server->sendError(PQ_ERRCODE_SERVER_BUSY);
      return;
   }

   if (request.scanning_pattern == 1)
   {
      server->sendError(PQ_ERRCODE_UNKNOWN_ERROR, "Scanning pattern not supported");
      return;
   }

   tcspc->getPreviewFLIMage()->setImageSize(request.n_x, request.n_y);

   if (request.measurement_type == PQ_MEASTYPE_TEST_IMAGESCAN)
   {
      setLive(true);
   }
   else if (request.measurement_type == PQ_MEASTYPE_IMAGESCAN)
   {
      file_writer->addMetadata("NumX", request.n_x);
      file_writer->addMetadata("NumY", request.n_y);
      file_writer->addMetadata("SpatialResolution_um", request.spatial_resolution);

      for(auto&& m : metadata)
         file_writer->addMetadata(m.first, m.second);

      acquireSequence();

      status_timer->start(1000); // TODO: should we start this for both types?
   }
   else
   {
      server->sendError(PQ_ERRCODE_UNKNOWN_ERROR);
   }

   assert(request.measurement_type == PQ_MEASTYPE_IMAGESCAN);
}

void FlimDisplay::processClientError(const QString error)
{
   QMessageBox::warning(this, "FLIM client error", error);
}

void FlimDisplay::processUserBreakRequest()
{
   status_timer->stop();

   if (tcspc->isLive())
      setLive(false);
   if (tcspc->acquisitionInProgress())
      stopSequence();
}

void FlimDisplay::sendStatusUpdate()
{
   E_PQ_MEAS_TYPE type = (tcspc->isLive()) ? PQ_MEASTYPE_TEST_IMAGESCAN : PQ_MEASTYPE_IMAGESCAN;
   std::map<QString, QVariant> data;

   auto rates = tcspc->getRates();
   data[QString('rate')] = rates.sync; // TODO: figure out what Leica wants here

   emit statusUpdate(type, data);
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

   preview_widget = new LifetimeDisplayWidget;
   preview_widget->setClosable(false);
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