#pragma once

#include "ThreadedObject.h"
#include <QDir>
#include <QVector>
#include <QApplication>
#include <QProcess>
#include <iostream>
#include <QJsonDocument>
#include <QJsonArray>
#include "TaskProgress.h"
#include "CustomDialog.h"

class BioformatsExporter : public ThreadedObject
{
public:

   BioformatsExporter(QFileInfo file, QObject* parent = nullptr) :
      ThreadedObject(parent), file(file), next_series(0), n_process_active(0)
   {
      QDir dir(QApplication::applicationDirPath());

      QString working_directory = QDir::toNativeSeparators(dir.absolutePath());
      QString bf_package = QDir::toNativeSeparators(dir.absoluteFilePath("bioformats_exporter.jar"));
      jar_path = QString("\"%1;%2\"").arg(working_directory).arg(bf_package);
      file_arg = QString("%1").arg(file.absoluteFilePath());

      initial_args << "-Xmx512m" << "-cp" << jar_path;

      getSeriesInformation();
   }


   static QStringList supportedExtensions()
   {
      return { "lif" };
   }


   void init()
   {
      int n_process = QThread::idealThreadCount();

      task = std::make_shared<TaskProgress>("Exporting series from lif file..", true, series.size());
      TaskRegister::addTask(task);
       
      n_process_active = n_process;
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
      QByteArray error_output = process->readAllStandardError();

      if (!error_output.isEmpty()) 
         throw std::runtime_error(error_output.toStdString());

      QJsonDocument doc = QJsonDocument::fromJson(output);
      QJsonArray series_json = doc.array();

      QStringList series_desc, series_name;

      for (int i = 0; i < series_json.count(); ++i) 
      {
         auto s = series_json.at(i);
         if (s.isObject())
         {
            QVariantMap map = s.toObject().toVariantMap();
            QString name = map["Name"].toString();
            QString desc = QString("%1 (%2x%3x%4x%5x%6)")
               .arg(name)
               .arg(map["SizeX"].toString())
               .arg(map["SizeY"].toString())
               .arg(map["SizeZ"].toString())
               .arg(map["SizeC"].toString())
               .arg(map["SizeT"].toString());
            series_desc.push_back(desc);
            series_name.push_back(name);
         }
         else
         {
            series_desc.push_back(QString("Series %1").arg(i, 3, 0));
         }
      }

      // Request from user
      CustomDialog d("Export series from lif", nullptr, BS_OKAY_ONLY);

      bool* export_series = new bool[series_desc.size()];

      if (series_desc.size() > 1)
      {
         d.addLabel("Please select series to export");
         for (int i = 0; i<series_desc.size(); i++)
         {
            export_series[i] = true;
            d.addCheckBox(QString("[%1] %2").arg(i).arg(series_desc[i]), &export_series[i]);
         }
         d.exec();

         series.clear();
         for (int i = 0; i < series_desc.size(); i++)
            if (export_series[i])
               series.push_back(QPair<int, QString>(i, series_name[i]));
      }

   }

   QProcess* runProcess(const QStringList& args, QProcess::ProcessChannelMode channel_mode = QProcess::ProcessChannelMode::SeparateChannels)
   {
      QProcess* process = new QProcess();
      process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
      process->setProcessChannelMode(channel_mode);
      process->start("java", args);
      return process;
   }


   void exportSeries(int series_idx, const QString series_name)
   {
      QString new_name = QString("%1 %2.ics").arg(file.baseName()).arg(series_name);
      QFileInfo new_file = QFileInfo(file.dir(), new_name);

      QStringList args = initial_args;
      args << "loci.formats.tools.ImageConverter" << "-overwrite" << "-series" << QString::number(series_idx) << QString("%2").arg(file.absoluteFilePath()) << QString("%3").arg(new_file.absoluteFilePath());

      QProcess* process = runProcess(args, QProcess::ForwardedChannels);
      connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &BioformatsExporter::processFinished);
      connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), process, &QProcess::deleteLater);
   }

   void processFinished(int exit_code, QProcess::ExitStatus status)
   {
      // TODO: some checking of exit code
      task->incrementStep();
      exportNext();

      if (n_process_active == 0)
         task->setFinished();
   }

   void exportNext()
   {
      int this_series = next_series++;
      if (this_series < series.size())
         exportSeries(series[this_series].first, series[this_series].second);
      else
         n_process_active--;
   }

private:

   QString jar_path;
   QFileInfo file;
   QStringList initial_args;
   QString file_arg;
   QVector<QPair<int,QString>> series;

   std::atomic<int> next_series;
   std::atomic<int> n_process_active;

   std::shared_ptr<TaskProgress> task;

};