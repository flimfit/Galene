#pragma once

#include "ui_MetaDataDialog.h"
#include "FLIMreader.h"

class MetaDataDialog : public QWidget, public Ui::MetaDataDialog
{
public:
   MetaDataDialog(const TagMap& tags, QWidget* parent = 0) : 
      QWidget(parent)
   {
      setupUi(this);

      QString text;

      for (auto t : tags)
         text.append(
            QString("%1: %2\n").arg(QString::fromStdString(t.first)).arg(QString::fromStdString(t.second.getString()))
         );
      
      metadata_edit->setText(text);
   }
};
