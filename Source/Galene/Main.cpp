#pragma warning(disable: 4503) // for ome-files code, produces over-long names

#include <QApplication>
#include <QCoreApplication>
#include "QSimpleUpdater.h"
#include "RealignmentStudio.h"
#include "FlimReaderDataSource.h"
#include "IntensityDataSource.h"
#include <boost/filesystem.hpp>
#include <ome/files/CoreMetadata.h>
#include "UiUtils.h"

const QString update_url = "http://galene.flimfit.org/updates.json";

int main(int argc, char *argv[])
{
   QFileInfo file(argv[0]);
   setenv("OME_HOME", qPrintable(file.absolutePath()), true);

   ome::common::setLogLevel(ome::logging::trivial::warning);

   qRegisterMetaType<std::shared_ptr<FlimReaderDataSource>>("std::shared_ptr<RealignableDataSource>");
   registerMetaTypes();

   QCoreApplication::setOrganizationName("FLIMfit");
   QCoreApplication::setOrganizationDomain("flimfit.org");
   QCoreApplication::setApplicationName("Galene");

   QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

   QApplication a(argc, argv);
   setDarkTheme(a);
   
   auto updater = QSimpleUpdater::getInstance();
   updater->setModuleVersion(update_url, VERSION); // from CMake
   updater->setModuleName(update_url, "Galene");
   updater->checkForUpdates(update_url);

   RealignmentStudio display;
   display.showMaximized();

   a.exec();

   return 0;
}