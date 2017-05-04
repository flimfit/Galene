
#include "FlimDisplay.h"
#include <QFileDialog>
#include <QVector>
#include <QDateTime>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include "FifoTcspcFactory.h"
#include "ConstrainedMdiSubWindow.h"
#include "FifoTcspcControlDisplayFactory.h"
#include "FlimFileReader.h"
#include "RealignmentStudio.h"
#include <functional>

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
   connect(generate_simulated_dataset_action, &QAction::triggered, this, &FlimDisplay::generateSimulatedDataset);

   file_writer = std::make_shared<FlimFileWriter>();
   connect(file_writer.get(), &FlimFileWriter::error, this, &FlimDisplay::displayErrorMessage);

   server = new FlimServer(this);
   status_timer = new QTimer(this);

   connect(server, &FlimServer::measurementRequest, this, &FlimDisplay::processMeasurementRequest);
   connect(server, &FlimServer::clientError, this, &FlimDisplay::processClientError);
   connect(server, &FlimServer::userBreakRequest, this, &FlimDisplay::processUserBreakRequest);
   connect(status_timer, &QTimer::timeout, this, &FlimDisplay::sendStatusUpdate);
   connect(this, &FlimDisplay::statusUpdate, server, &FlimServer::sendProgress);
   connect(this, &FlimDisplay::measurementRequestResponse, server, &FlimServer::sendMesurementRequestResponse);


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

   connect(realignment_action, &QAction::triggered, [&]() {
      auto m = new RealignmentStudio(); 
      m->showMaximized();
   });

   connect(workspace, &FlimWorkspace::openRequest, [&](const QString& filename) {
      try
      {
         FlimFileReader* reader = new FlimFileReader(filename);

         LifetimeDisplayWidget* widget = new LifetimeDisplayWidget;
         ConstrainedMdiSubWindow* sub = new ConstrainedMdiSubWindow();
         sub->setWidget(widget);
         sub->setAttribute(Qt::WA_DeleteOnClose);
         mdi_area->addSubWindow(sub);
         widget->setFlimDataSource(reader->getFLIMage());
         widget->show();
         widget->setWindowTitle(QFileInfo(filename).baseName());
      }
      catch (std::runtime_error e)
      {
         QMessageBox::warning(this, "Error loading file", QString("Could not load file '%1'").arg(filename));
      }
   });

}

void FlimDisplay::displayErrorMessage(const QString msg)
{
   QMessageBox::critical(this, "Error", msg);
}

void FlimDisplay::processMeasurementRequest(T_DATAFRAME_SRVREQUEST request, std::map<QString, QVariant> metadata)
{
   E_ERROR_CODES code = PQ_ERRCODE_NO_ERROR;

   if (!tcspc->readyForAcquisition())
      code = PQ_ERRCODE_UNKNOWN_ERROR;

   if (!workspace->hasWorkspace())
      code = PQ_ERRCODE_NO_WORKSPACE;

   if (code == PQ_ERRCODE_NO_ERROR)
   {
      auto flimage = tcspc->getPreviewFLIMage();
      flimage->setImageSize(request.n_x, request.n_y);
      flimage->setBidirectional(request.scanning_pattern);

      if (request.measurement_type == PQ_MEASTYPE_TEST_IMAGESCAN)
      {
         setLive(true);
         status_timer->start(1000);
      }
      else if (request.measurement_type == PQ_MEASTYPE_IMAGESCAN)
      {
         file_writer->addMetadata("NumX", request.n_x);
         file_writer->addMetadata("NumY", request.n_y);
         file_writer->addMetadata("SpatialResolution_um", request.spatial_resolution);
         file_writer->addMetadata("BidirectionalScan", (bool) request.scanning_pattern);

         QString filename = "";
         std::map<QString, QVariant>::iterator it;
         if ((it = metadata.find("Filename")) != metadata.end())
            filename = it->second.toString();
         else
            filename = "FLIM_";

         filename.remove(QRegExp("[/\\\\:\\.""\\*<>\\?|]")); // attempt to remove illegal characters - this isn't comprehensive


         for (auto&& m : metadata)
            file_writer->addMetadata(m.first, m.second);

         acquireSequenceImpl(filename, true); // use indeterminate acquisition
         status_timer->start(1000);
      }
      else
      {
         code = PQ_ERRCODE_UNKNOWN_ERROR;
      }
   }

   emit measurementRequestResponse(code);
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
   std::vector<std::pair<QString, QVariant>> data;

   auto flimage = tcspc->getPreviewFLIMage();
   auto rates = flimage->getCountRates();
   uint32_t maxcps = flimage->getMaxCountInPixel();

   auto add = [&](QString a, QVariant b) { data.push_back(std::make_pair(a, b)); };

   add("cps1", (uint32_t)0); // counts per sec on FCS channel 1
   add("cps2", (uint32_t)0); // counts per sec on FCS channel 2
   add("maxcpp", maxcps); // max counts per pixel

   for (int i = 0; i < rates.size(); i++)
      add(QString("det%1").arg(i+1), (uint32_t) rates[i]); // counts per sec on detector i
   add("ServerVersion", (uint32_t)0x05030203); // emulate SymphoTime 5.3.2.3
   
   emit statusUpdate(type, data);
}



void FlimDisplay::setupTCSPC()
{
   QString type = "sim";

   try 
   {
      tcspc = FifoTcspcFactory::create(type, this);
      tcspc_control = FifoTcspcControlDisplayFactory::create(tcspc);
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
   Bind(frame_accumulation_spin, tcspc, &FifoTcspc::setFrameAccumulation, &FifoTcspc::getFrameAccumulation, &FifoTcspc::frameAccumulationChanged);
   Bind(n_images_spin, tcspc, &FifoTcspc::setNumImages, &FifoTcspc::getNumImages);

   preview_widget = new LifetimeDisplayWidget;
   preview_widget->setClosable(false);
   ConstrainedMdiSubWindow* sub = new ConstrainedMdiSubWindow();
   sub->setWidget(preview_widget);
   sub->setAttribute(Qt::WA_DeleteOnClose);
   mdi_area->addSubWindow(sub);

   preview_widget->setFlimDataSource(tcspc->getPreviewFLIMage());
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

void FlimDisplay::shutdown()
{
}


void FlimDisplay::updateProgress(double progress)
{
   progress_bar->setValue(progress * 100);
}


void FlimDisplay::setLive(bool scanning)
{
   live_button->setChecked(scanning);
   acquire_sequence_button->setEnabled(!scanning);
   if (tcspc)
      tcspc->setLive(scanning);
}

void FlimDisplay::acquisitionStatusChanged(bool acq_in_progress, bool inderminate_acquisition)
{
   acquire_sequence_button->setEnabled(!acq_in_progress);
   live_button->setEnabled(!acq_in_progress);
   stop_button->setEnabled(acq_in_progress);
   progress_bar->setEnabled(acq_in_progress);

   if (!acq_in_progress)
      progress_bar->setValue(0);

   if (inderminate_acquisition)
      progress_bar->setMaximum(0);
   else
      progress_bar->setMaximum(100);
}

void FlimDisplay::acquireSequence()
{
   acquireSequenceImpl();
}

void FlimDisplay::acquireSequenceImpl(QString filename, bool indeterminate)
{
   if (tcspc == nullptr)
      return;

   try
   {      
      if (filename.isEmpty())
         filename = workspace->getNextFileName();
      else
         filename = workspace->getFileName(filename); // todo: add this to workspace
      file_writer->startRecording(filename);

      tcspc->startAcquisition(indeterminate);
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


void FlimDisplay::generateSimulatedDataset()
{
   std::thread t([&]() {
      
      double frequency = 0.01;

      double min_amplitude = 0;
      double max_amplitude = 200;
      int n_step = 10;

      double amplitude_step = (max_amplitude - min_amplitude) / (n_step - 1);

      int frame_accumulation = 25;

      tcspc->setFrameAccumulation(frame_accumulation);
      tcspc->setParameter("DisplacementFrequency", Float, frequency);
      
      
      for (int i = 0; i < n_step; i++)
      {
         while (tcspc->acquisitionInProgress())
            QThread::msleep(200);

         double amplitude = min_amplitude + i * amplitude_step;
         tcspc->setParameter("DisplacementAmplitude", Float, amplitude);

         QString filename = QString("Simulated Data Amplitude=%1 Frequency=%2 Frames=%3 ").arg(amplitude).arg(frequency).arg(frame_accumulation);
         filename.replace("\.", "_");
         workspace->setFilePrefix(filename);
            
         file_writer->addMetadata("DisplacementFrequency", frequency);
         file_writer->addMetadata("DisplacementAmplitude", amplitude);
         acquireSequence();
      }


   });
   t.detach();
}