#pragma once

#include <QApplication>
#include <QString>

#ifdef WIN32
int setenv(const char *name, const char *value, int overwrite);
#endif

void registerMetaTypes();
void setDarkTheme(QApplication& a);
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
