#pragma once
#ifdef USE_CRONOLOGIC

#include "cronologic.h"
#include "ParameterWidget.h"


class CronologicControlDisplay : public QWidget
{
   Q_OBJECT

public:
   explicit CronologicControlDisplay(Cronologic* tcspc, QWidget* parent = 0);

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

   Cronologic* tcspc;
   QList<ParameterWidget*> widgets;

};

#else

#include <QWidget>

class CronologicControlDisplay : public QWidget
{
   Q_OBJECT
};


#endif