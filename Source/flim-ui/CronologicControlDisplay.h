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

      // Ximea api provides no callback, so manually check all widgets when one value changes
      connect(w, &ParameterWidget::valueChanged, this, &CronologicControlDisplay::updateWidgets);
   }

   void updateWidgets()
   {
      for (auto& w : widgets)
         w->setWidgetValue();
   }

   void updateStreamingStatus(bool is_streaming);

   Cronologic* tcspc;
   QList<ParameterWidget*> widgets;

};

#endif