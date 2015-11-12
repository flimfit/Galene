#pragma once

#include <QWidget>
#include "BH.h"
#include "ui_BHRatesWidget.h"

class BHRatesWidget : public QWidget, public Ui::BHRatesWidget
{
public:

   BHRatesWidget(QWidget* parent) : QWidget(parent)
   {
      setupUi(this);
   };

   void SetRates(rate_values rates)
   {
      SYNC_bar->setValue(ConvertToLog(rates.sync_rate));
      CFD_bar->setValue(ConvertToLog(rates.cfd_rate));
      TAC_bar->setValue(ConvertToLog(rates.tac_rate));
      ADC_bar->setValue(ConvertToLog(rates.adc_rate));

      sync_rate_label->setText(CompactNumberString(rates.sync_rate));
      cfd_rate_label->setText(CompactNumberString(rates.cfd_rate));
      tac_rate_label->setText(CompactNumberString(rates.tac_rate));
      adc_rate_label->setText(CompactNumberString(rates.adc_rate));

   }

   void SetFifoUsage(float usage)
   {
      int usage_i = (int)(usage * 100.0);
      FIFO_bar->setValue(usage_i);
      fifo_label->setText(QString("%1%").arg(usage_i));
   }


private:

   double ConvertToLog(double rate)
   {
      return log10(rate) / log10(max_rate) * 100;
   }

   QString CompactNumberString(double rate)
   {
      if (rate <= 0.0)
         return "0.0e0";

      int ex = log10(rate);
      double m = rate / pow(10.0, ex);
      return QString("%1e%2").arg(m, 0, 'f', 1).arg(ex);
   }

   double max_rate = 80e6;
};