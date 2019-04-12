#pragma once

#include "FifoTcspc.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QProgressBar>
#include <QLabel>
#include <QMap>
#include <QLedIndicator.h>

class RateWidget : public QWidget
{
   Q_OBJECT

public:
   RateWidget(QWidget* parent, const QString& name);
   void setRate(float rate);

private:

   void setupUi(QWidget* parent);
   double convertToLog(double rate);
   QString compactNumberString(double rate);
   double max_rate = 80e6;

   QLabel* name_label;
   QLabel* rate_label;
   QProgressBar* rate_bar;

   QString name;
};

class WarningWidget : public QWidget
{
   Q_OBJECT

public:
   WarningWidget(QWidget* parent, const QString& name);
   void setStatus(FlimWarningStatus status);

private:

   void setupUi(QWidget* parent);

   QLabel* name_label;
   QLedIndicator* led;
   QString name;
};

class FlimStatusWidget : public QWidget
{
   Q_OBJECT

public:

   FlimStatusWidget(QWidget* parent);
   void setStatus(const FlimStatus& flim_status);

private:

   void setupUi();
   WarningWidget* getWarningWidget(const QString& name);
   RateWidget* getRateWidget(const QString& name);

   QVBoxLayout* main_layout;
   QVBoxLayout* warnings_layout;
   QVBoxLayout* rates_layout;
   QMap<QString, RateWidget*> rate_widgets;
   QMap<QString, WarningWidget*> warning_widgets;
};