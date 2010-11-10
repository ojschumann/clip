#include "qimagedataprovider.h"
#include <image/dataproviderfactory.h>
#include <ui/clip.h>

#include <iostream>

using namespace std;


QImageDataProvider::QImageDataProvider(const QImage& img, QObject *parent) :
    DataProvider(parent),
    data(img)
{

}


QImageDataProvider::DataProvider* QImageDataProvider::loadImage(const QString& filename) {
  cout << "QImageDP tries to load " << qPrintable(filename) << endl;
  QImage img(filename);
  if (!img.isNull()) {
    return new QImageDataProvider(img.convertToFormat(QImage::Format_ARGB32_Premultiplied));
  }
  return NULL;
}



const unsigned char* QImageDataProvider::getData() {
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


bool registerOK = DataProviderFactory::registerImageLoader(128, &QImageDataProvider::loadImage);
