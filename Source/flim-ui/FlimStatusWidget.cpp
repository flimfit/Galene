#include "FlimStatusWidget.h"

RateWidget::RateWidget(QWidget* parent, const QString& name) :
   QWidget(parent),
   name(name)
{
   setupUi(parent);
}

void RateWidget::setRate(float rate)
{
   rate_label->setText(compactNumberString(rate));
   rate_bar->setValue(convertToLog(rate));
}

void RateWidget::setupUi(QWidget* parent)
{
   setMinimumSize(50, 100);
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

   name_label = new QLabel();
   name_label->setText(name);
   name_label->setAlignment(Qt::AlignCenter);

   rate_label = new QLabel();
   rate_label->setText(0);
   rate_label->setAlignment(Qt::AlignCenter);

   rate_bar = new QProgressBar();
   rate_bar->setOrientation(Qt::Vertical);
   rate_bar->setValue(0);
   rate_bar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

   auto layout = new QVBoxLayout();
   layout->setContentsMargins(0, 0, 0, 0);
   layout->setSpacing(6);

   layout->addWidget(name_label);
   layout->addWidget(rate_label);
   layout->addWidget(rate_bar);

   setLayout(layout);
}

double RateWidget::convertToLog(double rate)
{
   return log10(rate) / log10(max_rate) * 100;
}

QString RateWidget::compactNumberString(double rate)
{
   if (rate <= 0.0)
      return "0.0e0";

   int ex = log10(rate);
   double m = rate / pow(10.0, ex);
   return QString("%1e%2").arg(m, 0, 'f', 1).arg(ex);
}



WarningWidget::WarningWidget(QWidget* parent, const QString& name) :
   QWidget(parent), name(name)
{
   setupUi(parent);
}

void WarningWidget::setStatus(FlimWarningStatus status)
{
   led->setStatus((LedStatus)status);
}


void WarningWidget::setupUi(QWidget* parent)
{
   name_label = new QLabel();
   name_label->setText(name);
   name_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

   led = new QLedIndicator();

   auto layout = new QHBoxLayout();
   layout->setContentsMargins(0, 0, 0, 0);
   layout->setSpacing(12);
   layout->addWidget(led);
   layout->addWidget(name_label);

   setLayout(layout);
}


FlimStatusWidget::FlimStatusWidget(QWidget* parent) : QWidget(parent)
{
   setupUi();
};

void FlimStatusWidget::setStatus(const FlimStatus& flim_status)
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

void FlimStatusWidget::setupUi()
{
   main_layout = new QVBoxLayout();
   warnings_layout = new QVBoxLayout();
   rates_layout = new QVBoxLayout();

   QGroupBox* warnings_box = new QGroupBox();
   warnings_box->setLayout(warnings_layout);
   warnings_box->setTitle("Status");

   QGroupBox* rates_box = new QGroupBox();
   rates_box->setLayout(rates_layout);
   rates_box->setTitle("Rates");

   main_layout->addWidget(warnings_box);
   main_layout->addWidget(rates_box);

   setLayout(main_layout);
}

RateWidget* FlimStatusWidget::getRateWidget(const QString& name)
{
   auto it = rate_widgets.find(name);
   if (it != rate_widgets.end())
      return it.value();

   auto w = new RateWidget(this, name);
   rates_layout->addWidget(w);
   rate_widgets[name] = w;
   return w;
}

WarningWidget* FlimStatusWidget::getWarningWidget(const QString& name)
{
   auto it = warning_widgets.find(name);
   if (it != warning_widgets.end())
      return it.value();

   auto w = new WarningWidget(this, name);
   warnings_layout->addWidget(w);
   warning_widgets[name] = w;
   return w;
}