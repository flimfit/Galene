#include "FlimDataSource.h"

FlimDataSource::FlimDataSource(QObject* parent) :
   QObject(parent)
{

}

void FlimDataSource::registerWatcher(FlimDataSourceWatcher* watcher)
{
   watchers.push_back(watcher);
}

void FlimDataSource::unregisterWatcher(FlimDataSourceWatcher* watcher)
{
   watchers.remove(watcher);
}

void FlimDataSource::requestDelete()
{
   for (auto& watcher : watchers)
      watcher->sourceDeleteRequested();
}
