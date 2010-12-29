#ifndef DATAPROVIDERFACTORY_H
#define DATAPROVIDERFACTORY_H

#include <QObject>
#include <QMultiMap>
#include <image/dataprovider.h>

class DataProviderFactory {
public:
  typedef DataProvider*(*DeviceOpener)(QObject*);

  static DataProviderFactory& getInstance();
  static bool registerImageLoader(int, DataProvider::ImageFactoryClass*);
  static bool registerDeviceOpener(int, DeviceOpener);

  DataProvider* loadImage(const QString&, QObject* = 0);

  QStringList registeredImageFormats();

private:
  explicit DataProviderFactory();
  DataProviderFactory(const DataProviderFactory&);
  ~DataProviderFactory();

  QMultiMap<int, DataProvider::ImageFactoryClass*> imageLoaders;


signals:

public slots:

};

#endif // DATAPROVIDERFACTORY_H
