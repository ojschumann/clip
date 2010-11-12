#include "dataproviderfactory.h"

#include <iostream>

using namespace std;


DataProviderFactory::DataProviderFactory(QObject *parent) :
    QObject(parent)
{
    cout << "init DataProviderFactory" << endl;
}

DataProviderFactory::DataProviderFactory(const DataProviderFactory &) {};

DataProviderFactory::~DataProviderFactory() {
  cout << "delete DataProviderFactory" << endl;
}

DataProviderFactory& DataProviderFactory::getInstance() {
  static DataProviderFactory instance;
  return instance;
}

DataProvider* DataProviderFactory::loadImage(const QString &filename, QObject* parent) {
  foreach (int key, imageLoaders.uniqueKeys()) {
    cout << "Searching in key " << key << endl;
    foreach (ImageLoader loader, imageLoaders.values(key)) {
      DataProvider* dp = (*loader)(filename, parent);
      if (dp) return dp;
    }
  }
  return NULL;
}

bool DataProviderFactory::registerImageLoader(int priority, ImageLoader loader) {
  DataProviderFactory::getInstance().imageLoaders.insert(priority, loader);
  return true;
}

bool DataProviderFactory::registerDeviceOpener(int, DeviceOpener) {
return true;
}

