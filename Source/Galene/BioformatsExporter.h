#pragma once

#include "ThreadedObject.h"
#include <QDir>
#include <QVector>
#include <QProcess>
#include "TaskProgress.h"
#include "CustomDialog.h"

class BioformatsExporter : public ThreadedObject
{
public:

   BioformatsExporter(QFileInfo file, QObject* parent = nullptr);
   static QStringList supportedExtensions();
   void init();
   
private:

   void getSeriesInformation();
   QProcess* runProcess(const QStringList& args, QProcess::ProcessChannelMode channel_mode = QProcess::ProcessChannelMode::SeparateChannels);
   void exportSeries(int series_idx, const QString series_name);
   void processFinished(int exit_code, QProcess::ExitStatus status);
   void exportNext();
      
   QString jar_path;
   QFileInfo file;
   QStringList initial_args;
   QString file_arg;
   QVector<QPair<int,QString>> series;

   std::atomic<int> next_series;
   std::atomic<int> n_process_active;

   std::shared_ptr<TaskProgress> task;
};