#pragma once

#include "SimTcspc.h"
#include "ParameterWidget.h"


class SimTcspcControlDisplay : public QWidget
{
   Q_OBJECT

public:
   explicit SimTcspcControlDisplay(SimTcspc* tcspc, QWidget* parent = 0);

private:

   /*
   Add a CronologicControlWidget

   The arguments to this function should match those to the ParameterControlWidget
   function excluding the inital pointer
   */
   template<typename... Args>
   void addWidget(Args... args)
   {
      ParameterWidget* w = new ParameterWidget(tcspc, args...);
      widgets.push_back(w);

      connect(w, &ParameterWidget::valueChanged, this, &CronologicControlDisplay::updateWidgets);
   }

   void updateWidgets()
   {
      for (auto& w : widgets)
         w->setWidgetValue();
   }

   SimTcspc* tcspc;
   QList<ParameterWidget*> widgets;

};
