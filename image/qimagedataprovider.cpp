#include "qimagedataprovider.h"
#include <image/dataproviderfactory.h>
#include <ui/clip.h>

#include <iostream>

using namespace std;


QImageDataProvider::QImageDataProvider(const QImage& img, QObject *parent) :
    DataProvider(parent),
    data(img)
{
  cout << "init QImageDataProvider" << endl;
}

QImageDataProvider::~QImageDataProvider() {
  cout << "delete QImageDataProvider" << endl;
}

QImageDataProvider::DataProvider* QImageDataProvider::loadImage(const QString& filename, QObject* parent) {
  cout << "QImageDP tries to load " << qPrintable(filename) << endl;
  QImage img(filename);
  if (!img.isNull()) {
    foreach (QString key, img.textKeys()) {
      cout << qPrintable(key) << " -> " << qPrintable(img.text(key)) << endl;
    }

    return new QImageDataProvider(img.convertToFormat(QImage::Format_ARGB32_Premultiplied), parent);
  }
  return NULL;
}



const void* QImageDataProvider::getData() {
  return data.bits();
}

int QImageDataProvider::width() {
  return data.width();
}

int QImageDataProvider::height() {
  return data.height();
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

bool registerOK = DataProviderFactory::registerImageLoader(128, &QImageDataProvider::loadImage);
