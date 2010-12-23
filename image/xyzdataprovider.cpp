#include "xyzdataprovider.h"

#include <QFile>
#include <QDataStream>
#include <iostream>

#include "image/dataproviderfactory.h"

using namespace std;

XYZDataProvider::XYZDataProvider(QObject *parent) :
    DataProvider(parent)
{
  cout << "init XYZDataProvider" << endl;
}

XYZDataProvider::~XYZDataProvider() {
  cout << "delete XYZDataProvider" << endl;
}

DataProvider* XYZDataProvider::loadImage(const QString& filename, QObject* parent) {
  cout << "XyzDP tries to load " << qPrintable(filename) << endl;

  QFile imgFile(filename);

  if (!imgFile.open(QFile::ReadOnly)) return NULL;

  QDataStream in(&imgFile);
  in.setByteOrder(QDataStream::LittleEndian);

  short unsigned int tmp;

  in >> tmp;
  int width = tmp;
  in >> tmp;
  int height = tmp;

  QVector<float> pixelData;
  pixelData.reserve(width*height);

  if ((2*width*height+4)!=imgFile.size()) return NULL;

  for (int i=0; i<width*height; i++) {
    in >> tmp;
    pixelData << static_cast<float>(tmp);
  }

  XYZDataProvider* provider = new XYZDataProvider(parent);
  provider->pixelData = pixelData;
  provider->imgWidth = width;
  provider->imgHeight = height;
  provider->insertFileInformation(filename);
  provider->providerInformation.insert("Size", QString("%1x%2").arg(width).arg(height));

  return provider;
}


const void* XYZDataProvider::getData() {
  return (void*)pixelData.data();
}

int XYZDataProvider::width() {
  return imgWidth;
}

int XYZDataProvider::height() {
  return imgHeight;
}

int XYZDataProvider::bytesCount() {
  return pixelData.size()*sizeof(float);
}

int XYZDataProvider::pixelCount() {
  return pixelData.size();
}

DataProvider::Format XYZDataProvider::format() {
  return Float32;
}

void XYZDataProvider::saveToXML(QDomElement) {

}

void XYZDataProvider::loadFromXML(QDomElement) {

}

bool XYZRegisterOK = DataProviderFactory::registerImageLoader(192, &XYZDataProvider::loadImage);
