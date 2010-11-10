#include "dataproviderfactory.h"

#include <iostream>

using namespace std;


DataProviderFactory::DataProviderFactory(QObject *parent) :
    QObject(parent)
{
}

DataProviderFactory::DataProviderFactory(const DataProviderFactory &) {};

DataProviderFactory::~DataProviderFactory() {}

DataProviderFactory& DataProviderFactory::getInstance() {
  static DataProviderFactory instance;
  return instance;
}

DataProvider* DataProviderFactory::loadImage(const QString &filename) {
  foreach (int key, imageLoaders.uniqueKeys()) {
    cout << "Searching in key " << key << endl;
    foreach (ImageLoader loader, imageLoaders.values(key)) {
      DataProvider* dp = (*loader)(filename);
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

