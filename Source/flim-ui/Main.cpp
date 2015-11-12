#include <QApplication>
#include <QCoreApplication>
#include "FlimDisplay.h"
#include "BHRatesWidget.h"
#include "HighPerformanceTimer.h"
#include <memory>
#include <iostream>

HighPerformanceTimer sys_timer("System");

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)

{
   QByteArray localMsg = msg.toLocal8Bit();

   switch (type)
   {
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
   qRegisterMetaType<rate_values>("rate_values");

   QCoreApplication::setOrganizationName("Garvan Institute of Medical Research");
   QCoreApplication::setOrganizationDomain("garvan.org.au");
   QCoreApplication::setApplicationName("FifoFlim");

   qInstallMessageHandler(myMessageOutput);

   QApplication a(argc, argv);
       
   FlimDisplay display;
   display.showMaximized();

   a.exec();

   return 0;
}