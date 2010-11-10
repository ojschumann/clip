#include "basdataprovider.h"
#include <image/dataproviderfactory.h>

#include <iostream>

using namespace std;

BasDataProvider::BasDataProvider(QObject *parent) :
    DataProvider(parent)
{
}

DataProvider* BasDataProvider::loadImage(const QString& filename) {
  cout << "BasDP tries to load " << qPrintable(filename) << endl;
  return NULL;
}

const unsigned char* BasDataProvider::getData() {
  return NULL;
}

int BasDataProvider::width() {
  return 0;
}

int BasDataProvider::height() {
  return 0;
}

int BasDataProvider::bytesCount() {
  return 0;
}

int BasDataProvider::pixelCount() {
  return 0;
}

bool BasRegisterOK = DataProviderFactory::registerImageLoader(0, &BasDataProvider::loadImage);
