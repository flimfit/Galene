#pragma once

#include <QMainWindow>
#include <QDir>
#include <QFileDialog>
#include <QAbstractListModel>
#include "qcustomplot.h"

#include "ControlBinder.h"
#include "Oscilloscope.h"
#include "ImageRenderWindow.h"
#include "LifetimeDisplayWidget.h"
#include "ui_FlimDisplay.h"
#include "SimTcspc.h"
#include <memory>


class FlimWorkspace : public QAbstractListModel
{
   Q_OBJECT

public:
   FlimWorkspace(QObject* parent = 0, const QString& dir = "") :
      QAbstractListModel(parent)
   {
      if (dir != "")
      {
         if (QDir(dir).exists())
            openFromFolder(dir);
         else
            makeNewFromFolder(dir);
      }
   }

   void update()
   {
      if (has_workspace)
      {
         beginResetModel();
         files = workspace.entryList({ "*.ffd" }, QDir::NoFilter, QDir::Time);
         endResetModel();
      }
   }

   int rowCount(const QModelIndex & parent = QModelIndex()) const
   {
      return files.length();
   }

   QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const
   {
      if (role == Qt::DisplayRole)
      {
         if (index.row() < files.size())
            return files[index.row()];
      }

      return QVariant();
   }

   void makeNew()
   {
      QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
      QString new_dir = QFileDialog::getSaveFileName(nullptr, "Choose a workspace", folder, "");
 
      makeNewFromFolder(new_dir);
   }

   void open()
   {
      QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
      QString open_dir = QFileDialog::getExistingDirectory(nullptr, "Choose a workspace", folder);

      openFromFolder(open_dir);
   }

   void makeNewFromFolder(const QString& dir)
   {
      if (!QDir(dir).exists())
         QDir().mkdir(dir);
      
      workspace = dir;
      has_workspace = true;
      update();
   }

   void openFromFolder(const QString& dir)
   {
      workspace = dir;
      has_workspace = true;
      update();
   }

   QString getNextFileName()
   {
      if (!has_workspace)
         throw std::runtime_error("No workspace selected. Please select a workspace from the file menu.");

      QString next_file = workspace.absoluteFilePath(QString("%1_%2.ffd").arg(file_prefix).arg(sequence_number, 3, 10, QChar('0')));
      
      emit sequenceNumberChanged(++sequence_number);
      update();
      return next_file;
   }

   void setFilePrefix(const QString& prefix_) { file_prefix = prefix_; }
   const QString& getFilePrefix() { return file_prefix; }

   void setSequenceNumber(int sequence_number_) { sequence_number = sequence_number_; }
   int getSequenceNumber() { return sequence_number; }

signals:
   void sequenceNumberChanged(int sequence_number);

protected:

   void enumerate()
   {

   }

   bool has_workspace = false;
   QDir workspace;
   QString file_prefix = "FLIM_Data_";
   int sequence_number = 0;

   int n_files;
   QStringList files;
};


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

   FlimWorkspace* workspace;
};
