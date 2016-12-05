#include <QApplication>
#include <QCoreApplication>
#include "QSimpleUpdater.h"
#include "FlimDisplay.h"
#include "RealignmentStudio.h"
#include "BHRatesWidget.h"
#include "FlimReaderDataSource.h"
#include <memory>
#include <vector>
#include <iostream>

const QString update_url = "https://seanwarren.github.io/flim-ui-website/updates.json";

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
   qRegisterMetaType<cv::Point2d>("cv::Point2d");
   qRegisterMetaType<FlimRates>("FlimRates");
   qRegisterMetaType<std::vector<int64_t>>("std::vector<int32_t>");
   qRegisterMetaType<std::vector<uint64_t>>("std::vector<uint32_t>");
   qRegisterMetaType<std::vector<double>>("std::vector<double>");
   qRegisterMetaType<T_DATAFRAME_SRVREQUEST>("T_DATAFRAME_SRVREQUEST");
   qRegisterMetaType<E_ERROR_CODES>("E_ERROR_CODES");
   qRegisterMetaType<E_PQ_MEAS_TYPE>("E_PQ_MEAS_TYPE");
   qRegisterMetaType<std::map<QString,QVariant>>("std::map<QString, QVariant>");
   qRegisterMetaType<std::vector<std::pair<QString, QVariant>>>("std::vector<std::pair<QString, QVariant>>");
   qRegisterMetaType<std::shared_ptr<FlimReaderDataSource>>("std::shared_ptr<FlimReaderDataSource>");

   QCoreApplication::setOrganizationName("Garvan Institute of Medical Research");
   QCoreApplication::setOrganizationDomain("garvan.org.au");
   QCoreApplication::setApplicationName("FifoFlim");

   qInstallMessageHandler(myMessageOutput);

   QApplication a(argc, argv);

   
   auto updater = QSimpleUpdater::getInstance();
   updater->setModuleVersion(update_url, VERSION); // from CMake
   updater->setModuleName(update_url, "flim-ui");
   updater->checkForUpdates(update_url);
   

   //FlimDisplay display;
   RealignmentStudio display;
   display.showMaximized();

   a.exec();

   return 0;
}