#include "dataprovider.h"

DataProvider::DataProvider(QObject *parent) :
    QObject(parent)
{

}

DataProvider* DataProvider::openDevice() {
  return NULL;
}

DataProvider* DataProvider::loadImage(const QString &) {
  return NULL;
}
