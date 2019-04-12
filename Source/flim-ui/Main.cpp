#pragma warning(disable: 4503) // for ome-files code, produces over-long names

#include "UiUtils.h"
#include <QApplication>
#include <QCoreApplication>
#include "QSimpleUpdater.h"
#include "FlimDisplay.h"
#include "FlimStatusWidget.h"

const QString update_url = "http://galene.flimfit.org/updates.json";

int main(int argc, char *argv[])
{
   qRegisterMetaType<FlimStatus>("FlimStatus");
   qRegisterMetaType<T_DATAFRAME_SRVREQUEST>("T_DATAFRAME_SRVREQUEST");
   qRegisterMetaType<E_ERROR_CODES>("E_ERROR_CODES");
   qRegisterMetaType<E_PQ_MEAS_TYPE>("E_PQ_MEAS_TYPE");

   registerMetaTypes();

   QCoreApplication::setOrganizationName("FLIMfit");
   QCoreApplication::setOrganizationDomain("flimfit.org");
   QCoreApplication::setApplicationName("flim-ui");

   QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");


   QApplication a(argc, argv);

   setDarkTheme(a);

   auto updater = QSimpleUpdater::getInstance();
   updater->setModuleVersion(update_url, VERSION); // from CMake
   updater->setModuleName(update_url, "flim-ui");
   updater->checkForUpdates(update_url);

   FlimDisplay display;
   display.showMaximized();

   a.exec();

   return 0;
}