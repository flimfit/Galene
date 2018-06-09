#pragma once

#include "ThreadedObject.h"
#include <QDir>
#include <QApplication>
#include <QProcess>
#include <iostream>
#include <QJsonDocument>
#include <QJsonArray>
#include "TaskProgress.h"

class BioformatsExporter : public ThreadedObject
{
public:

   BioformatsExporter(QFileInfo file, QObject* parent = nullptr) :
      ThreadedObject(parent), file(file)
   {
      QDir dir(QApplication::applicationDirPath());
      dir.cd("bftools");

      QString working_directory = QDir::toNativeSeparators(dir.absolutePath());
      QString bf_package = QDir::toNativeSeparators(dir.absoluteFilePath("bioformats_exporter.jar"));
      jar_path = QString("%1;%2").arg(working_directory).arg(bf_package);
      file_arg = QString("%1").arg(file.absoluteFilePath());

      initial_args << "-Xmx512m" << "-cp" << jar_path;

      next_series = 0;
   }

   void init()
   {
      getSeriesInformation();

      int n_process = QThread::idealThreadCount();

      for (int i = 0; i < n_process; i++)
         exportNext();
   }

   void getSeriesInformation()
   {
      QStringList args = initial_args;
      args << "loci.formats.tools.SeriesInfo" << file_arg;
      QProcess* process = runProcess(args);

      process->waitForFinished(10000);
      QByteArray output = process->readAllStandardOutput();

      QJsonDocument doc = QJsonDocument::fromJson(output);
      QJsonArray series = doc.array();

      n_series = series.count();
      series_names.clear();

      for (int i = 0; i < n_series; ++i) 
      {
         auto s = series.at(i);
         if (s.isObject())
         {
            QVariantMap map = s.toObject().toVariantMap();
            QString name = map["Name"].toString();
            series_names.push_back(name);
         }
         else
         {
            series_names.push_back(QString("Series %1").arg(i, 3, 0));
         }
      }

      task = std::make_shared<TaskProgress>("Exporting...", true, n_series);
      TaskRegister::addTask(task);
   }

   QProcess* runProcess(const QStringList& args, QProcess::ProcessChannelMode channel_mode = QProcess::ProcessChannelMode::SeparateChannels)
   {
      QProcess* process = new QProcess();
      process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
      process->setProcessChannelMode(channel_mode);
      process->start("java", args);
      return process;
   }


   void exportSeries(int series)
   {
         QString new_name = QString("%1 %2.ics").arg(file.baseName()).arg(series_names[series]);
         QFileInfo new_file = QFileInfo(file.dir(), new_name);

         QStringList args = initial_args;
         args << "loci.formats.tools.ImageConverter" << "-overwrite" << "-series" << QString::number(series) << QString("%2").arg(file.absoluteFilePath()) << QString("%3").arg(new_file.absoluteFilePath());

         QProcess* process = runProcess(args, QProcess::ForwardedChannels);
         connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &BioformatsExporter::processFinished);
         connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), process, &QProcess::deleteLater);
   }

   void processFinished(int exit_code, QProcess::ExitStatus status)
   {
      // TODO: some checking of exit code
      task->incrementStep();
      exportNext();
   }

   void exportNext()
   {
      int series = next_series++;
      if (series < n_series)
         exportSeries(series);
   }

private:

   QString jar_path;
   QFileInfo file;
   QStringList initial_args;
   QString file_arg;
   QStringList series_names;

   int n_series;
   std::atomic<int> next_series;

   std::shared_ptr<TaskProgress> task;

};