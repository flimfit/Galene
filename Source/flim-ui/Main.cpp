#pragma warning(disable: 4503) // for ome-files code, produces over-long names

#include <QApplication>
#include <QCoreApplication>
#include "QSimpleUpdater.h"
#include "FlimDisplay.h"
#include "RealignmentStudio.h"
#include "BHRatesWidget.h"
#include "FlimReaderDataSource.h"
#include "IntensityDataSource.h"
#include <memory>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <ome/files/CoreMetadata.h>

const QString update_url = "https://seanwarren.github.io/flim-ui-website/updates.json";

#ifdef WIN32
int setenv(const char *name, const char *value, int overwrite)
{
   int errcode = 0;
   if (!overwrite) {
      size_t envsize = 0;
      errcode = getenv_s(&envsize, NULL, 0, name);
      if (errcode || envsize) return errcode;
   }
   return _putenv_s(name, value);
}
#endif


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)

{
   QByteArray localMsg = msg.toLocal8Bit();

   switch (type)
   {
   case QtInfoMsg:
      fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;
      
   case QtDebugMsg:
      fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;

   case QtWarningMsg:
      fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;

   case QtCriticalMsg:
      fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;

   case QtFatalMsg:
      fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      abort();
         
   }
}

int main(int argc, char *argv[])
{

   QFileInfo file(argv[0]);
   setenv("OME_HOME", qPrintable(file.absolutePath()), true);

   ome::common::setLogLevel(ome::logging::trivial::warning);

   qRegisterMetaType<cv::Point2d>("cv::Point2d");
   qRegisterMetaType<FlimRates>("FlimRates");
   qRegisterMetaType<std::vector<int64_t>>("std::vector<int32_t>");
   qRegisterMetaType<std::vector<uint64_t>>("std::vector<uint32_t>");
   qRegisterMetaType<std::vector<double>>("std::vector<double>");
   qRegisterMetaType<T_DATAFRAME_SRVREQUEST>("T_DATAFRAME_SRVREQUEST");
   qRegisterMetaType<E_ERROR_CODES>("E_ERROR_CODES");
   qRegisterMetaType<E_PQ_MEAS_TYPE>("E_PQ_MEAS_TYPE");
   qRegisterMetaType<std::map<QString, QVariant>>("std::map<QString, QVariant>");
   qRegisterMetaType<std::vector<std::pair<QString, QVariant>>>("std::vector<std::pair<QString, QVariant>>");
   qRegisterMetaType<std::shared_ptr<FlimReaderDataSource>>("std::shared_ptr<RealignableDataSource>");

   QCoreApplication::setOrganizationName("FLIMfit");
   QCoreApplication::setOrganizationDomain("flimfit.org");
   QCoreApplication::setApplicationName("Galene");

   QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

   //qInstallMessageHandler(myMessageOutput);


   QApplication a(argc, argv);

   a.setStyle(QStyleFactory::create("Fusion"));

   
   QPalette darkPalette;
   darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
   darkPalette.setColor(QPalette::WindowText, Qt::white);
   darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
   darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
   darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
   darkPalette.setColor(QPalette::ToolTipText, Qt::white);
   darkPalette.setColor(QPalette::Text, Qt::white);
   darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
   darkPalette.setColor(QPalette::ButtonText, Qt::white);
   darkPalette.setColor(QPalette::BrightText, Qt::red);
   darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

   darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
   darkPalette.setColor(QPalette::HighlightedText, Qt::black);

   a.setPalette(darkPalette);

   a.setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
   

   auto updater = QSimpleUpdater::getInstance();
   updater->setModuleVersion(update_url, "VERSION"); // from CMake
   updater->setModuleName(update_url, "flim-ui");
   updater->checkForUpdates(update_url);
   

   //FlimDisplay display;
   RealignmentStudio display;
   display.showMaximized();

   a.exec();

   return 0;
}