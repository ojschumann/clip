#include "qimagedataprovider.h"

#include <QStringList>
#include <QMap>
#include <iostream>
#include <QImageReader>

#include "image/dataproviderfactory.h"
#include "image/imagedatastore.h"

using namespace std;


QImageDataProvider::QImageDataProvider(const QImage& img, QObject *parent) :
    DataProvider(parent),
    data(img)
{
}

QImageDataProvider::~QImageDataProvider() {
}

QStringList QImageDataProvider::Factory::fileFormatFilters() {
  QStringList formats;
  foreach (QByteArray format, QImageReader::supportedImageFormats()) {
    formats += QString(format);
  }
  return formats;
}

DataProvider* QImageDataProvider::Factory::getProvider(QString filename, ImageDataStore *store, QObject *parent) {
  QImage img(filename);
  if (!img.isNull()) {
    QMap<QString, QVariant> headerData;
    foreach (QString key, img.textKeys()) {
      if (key!="")
        headerData.insert(key, QVariant(img.text(key)));
    }

    store->setData(ImageDataStore::PixelSize, img.size());

    headerData.insert("Size", QString("%1x%2").arg(img.width()).arg(img.height()));
    QImageDataProvider* provider = new QImageDataProvider(img.convertToFormat(QImage::Format_ARGB32_Premultiplied), parent);
    provider->insertFileInformation(filename);
    provider->providerInformation.unite(headerData);
    return provider;
  }
  return NULL;
}

const void* QImageDataProvider::getData() {
  return data.bits();
}

QSize QImageDataProvider::size() {
  return data.size();
}

int QImageDataProvider::bytesCount() {
  return data.byteCount();
}

int QImageDataProvider::pixelCount() {
  return data.width()*data.height();
}

DataProvider::Format QImageDataProvider::format() {
  return RGB8Bit;
}

void QImageDataProvider::saveToXML(QDomElement) {

}

void QImageDataProvider::loadFromXML(QDomElement) {

}

bool registerOK = DataProviderFactory::registerImageLoader(128, new QImageDataProvider::Factory());
