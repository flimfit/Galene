#pragma once

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QTimer>
#include "qcustomplot.h"

#include "ControlBinder.h"
#include "Oscilloscope.h"
#include "ImageRenderWindow.h"
#include "LifetimeDisplayWidget.h"
#include "SimTcspc.h"
#include "FlimFileWriter.h"
#include "FlimWorkspace.h"
#include "FlimServer.h"
#include "LiveFlimDataSource.h"

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
   void showTcspcSettings();

   void openFile(const QString& filename);
   void generateSimulatedDataset();

signals:
   void statusUpdate(E_PQ_MEAS_TYPE measurement_type, std::vector<std::pair<QString, QVariant>> optional_data);
   void measurementRequestResponse(E_ERROR_CODES code);

protected:

   void workspaceOpened(const QString& workspace);

   void processMeasurementRequest(T_DATAFRAME_SRVREQUEST request, std::map<QString, QVariant> metadata);
   void processClientError(const QString);
   void processUserBreakRequest();
   void sendStatusUpdate();


private:

   void setupTCSPC();
   void acquireSequence();
   void acquireSequenceImpl(QString filename = "", bool indeterminate = false);
   void stopSequence();

   void acquisitionStatusChanged(bool acq_in_progress, bool indeterminate);

   void displayErrorMessage(const QString error);

   void updateFlimStatus();

   void updateSizes(int index = -1);

   ImageRenderWindow* flim_display = nullptr;

   QWidget* tcspc_control = nullptr;
   QMdiSubWindow* tcspc_settings_window = nullptr;
   FifoTcspc* tcspc = nullptr;
   LiveFlimDataSource* flim_data_source;

   std::shared_ptr<FlimFileWriter> file_writer;
   LifetimeDisplayWidget* preview_widget;
   FlimWorkspace* workspace;
   FlimServer* server;

   QTimer* status_timer;
   QTimer* update_timer; 
};
