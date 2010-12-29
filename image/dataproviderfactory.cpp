#include "dataproviderfactory.h"

#include <iostream>
#include <QStringList>

using namespace std;


DataProviderFactory::DataProviderFactory()
{
    cout << "init DataProviderFactory" << endl;
}

DataProviderFactory::DataProviderFactory(const DataProviderFactory &) {};

DataProviderFactory::~DataProviderFactory() {
  foreach (DataProvider::ImageFactoryClass* item, imageLoaders.values()) {
    delete item;
  }
  cout << "delete DataProviderFactory" << endl;
}

DataProviderFactory& DataProviderFactory::getInstance() {
  static DataProviderFactory instance;
  return instance;
}

DataProvider* DataProviderFactory::loadImage(const QString &filename, QObject* parent) {
  foreach (int key, imageLoaders.uniqueKeys()) {
    cout << "Searching in key " << key << endl;
    foreach (DataProvider::ImageFactoryClass* loader, imageLoaders.values(key)) {
      DataProvider* dp = loader->getProvider(filename, parent);
      if (dp) return dp;
    }
  }
  return NULL;
}

QStringList DataProviderFactory::registeredImageFormats() {
  QStringList formats;
  foreach (int key, imageLoaders.uniqueKeys()) {
    foreach (DataProvider::ImageFactoryClass* loader, imageLoaders.values(key)) {
      formats += loader->fileFormatFilters();
    }
  }
  return formats;
}

bool DataProviderFactory::registerImageLoader(int priority, DataProvider::ImageFactoryClass* loader) {
  DataProviderFactory::getInstance().imageLoaders.insert(priority, loader);
  return true;
}

bool DataProviderFactory::registerDeviceOpener(int, DeviceOpener) {
return true;
}

