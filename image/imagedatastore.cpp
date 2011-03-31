#include "imagedatastore.h"

ImageDataStore::ImageDataStore(QObject *parent) :
    QObject(parent)
{
}


bool ImageDataStore::hasData(DataType d) const {
  bool b = dataStore.contains(d);
  return dataStore.contains(d);
}

double ImageDataStore::getData(DataType d) const {
  return dataStore.value(d, 0.0);
}

void ImageDataStore::setData(DataType d, double v) {
  if (!hasData(d) || getData(d)!=v) {
    dataStore[d]=v;
    emit dataChanged(d, v);
  }
}
