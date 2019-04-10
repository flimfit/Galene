#include "UiUtils.h"

#include <memory>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <opencv2/core/core.hpp>

#include <QtPlugin>
#include <QMetaType>
#include <QPalette>
#include <QStyleFactory>

#ifdef __APPLE__
   Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#endif
 
#ifdef WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
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


void registerMetaTypes()
{
   qRegisterMetaType<cv::Point2d>("cv::Point2d");
   qRegisterMetaType<std::vector<int64_t>>("std::vector<int32_t>");
   qRegisterMetaType<std::vector<uint64_t>>("std::vector<uint32_t>");
   qRegisterMetaType<std::vector<double>>("std::vector<double>");
   qRegisterMetaType<std::map<QString, QVariant>>("std::map<QString, QVariant>");
   qRegisterMetaType<std::vector<std::pair<QString, QVariant>>>("std::vector<std::pair<QString, QVariant>>");

}

void setDarkTheme(QApplication& a)
{
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
}

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