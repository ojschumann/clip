#include "dataprovider.h"
#include <iostream>

using namespace std;

DataProvider::DataProvider(QObject *parent) :
    QObject(parent)
{
  cout << "init DataProvider" << endl;
}

DataProvider* DataProvider::openDevice() {
  return NULL;
}

DataProvider* DataProvider::loadImage(const QString &) {
  return NULL;
}
