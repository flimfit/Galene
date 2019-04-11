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
   RateWidget(QWidget* parent, const QString& name) : 
      QWidget(parent),
      name(name)
   {
      setupUi(parent);
   }

   void setRate(float rate)
   {
      rate_label->setText(compactNumberString(rate));
      rate_bar->setValue(convertToLog(rate));
   }

private:

   void setupUi(QWidget* parent)
   {
      name_label = new QLabel(parent);
      name_label->setText(name);
      name_label->setAlignment(Qt::AlignCenter);

      rate_label = new QLabel(parent);
      rate_label->setText(0);
      rate_label->setAlignment(Qt::AlignCenter);

      rate_bar = new QProgressBar(parent);
      rate_bar->setOrientation(Qt::Vertical);
      rate_bar->setValue(0);

      auto layout = new QVBoxLayout(parent);
      layout->addWidget(name_label);
      layout->addWidget(rate_label);
      layout->addWidget(rate_bar);

      setLayout(layout);
   }

   double convertToLog(double rate)
   {
      return log10(rate) / log10(max_rate) * 100;
   }

   QString compactNumberString(double rate)
   {
      if (rate <= 0.0)
         return "0.0e0";

      int ex = log10(rate);
      double m = rate / pow(10.0, ex);
      return QString("%1e%2").arg(m, 0, 'f', 1).arg(ex);
   }

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
   WarningWidget(QWidget* parent, const QString& name) :
      QWidget(parent), name(name)
   {
      setupUi(parent);
   }

   void setStatus(FlimWarningStatus status)
   {
      led->setStatus((LedStatus)status);
   }

private:

   void setupUi(QWidget* parent)
   {
      name_label = new QLabel(parent);
      name_label->setText(name);
      name_label->setAlignment(Qt::AlignLeft);

      led = new QLedIndicator(parent);

      auto layout = new QHBoxLayout(parent);
      layout->addWidget(led);
      layout->addWidget(name_label);

      setLayout(layout);
   }

   QLabel* name_label;
   QLedIndicator* led;
   QString name;
};

class BHRatesWidget : public QWidget //, public Ui::BHRatesWidget
{
   Q_OBJECT

public:

   BHRatesWidget(QWidget* parent) : QWidget(parent)
   {
      setupUi();
   };

   void update(const FlimStatus& flim_status)
   {
      for (auto it = flim_status.rates.constKeyValueBegin(); it != flim_status.rates.constKeyValueEnd(); it++)
      {
         auto pair = *it;
         getRateWidget(pair.first)->setRate(pair.second);
      }

      for (auto it = flim_status.warnings.constKeyValueBegin(); it != flim_status.warnings.constKeyValueEnd(); it++)
      {
         auto pair = *it;
         getWarningWidget(pair.first)->setStatus(pair.second.getStatus());
      }
   }

   void setupUi()
   {
      main_layout = new QVBoxLayout(this);
      warnings_layout = new QVBoxLayout(this);
      rates_layout = new QVBoxLayout(this);

      QGroupBox* warnings_box = new QGroupBox(this);
      warnings_box->setLayout(warnings_layout);
      warnings_box->setTitle("Status");

      QGroupBox* rates_box = new QGroupBox(this);   
      rates_box->setLayout(rates_layout);
      rates_box->setTitle("Rates");

      main_layout->addWidget(warnings_box);
      main_layout->addWidget(rates_box);
   }

   RateWidget* getRateWidget(const QString& name)
   {
      auto it = rate_widgets.find(name);
      if (it != rate_widgets.end())
         return it.value();

      auto w = new RateWidget(this, name);
      rates_layout->addWidget(w);
      rate_widgets[name] = w;
      return w;
   }
   
   WarningWidget* getWarningWidget(const QString& name)
   {
      auto it = warning_widgets.find(name);
      if (it != warning_widgets.end())
         return it.value();

      auto w = new WarningWidget(this, name);
      warnings_layout->addWidget(w);
      warning_widgets[name] = w;
      return w;
   }
private:

   QVBoxLayout* main_layout;
   QVBoxLayout* warnings_layout;
   QVBoxLayout* rates_layout;
   QMap<QString, RateWidget*> rate_widgets;
   QMap<QString, WarningWidget*> warning_widgets;
};