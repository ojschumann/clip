#include "imagedatastore.h"

#include <QDebug>

#include "tools/tools.h"

ImageDataStore::ImageDataStore(QObject *parent) :
    QObject(parent)
{
}


bool ImageDataStore::hasData(DataType d) const {
  return dataStore.contains(d);
}

QVariant ImageDataStore::getData(DataType d) const {
  return dataStore.value(d, 0.0);
}

void ImageDataStore::setData(DataType d, QVariant v) {
  if (!hasData(d) || getData(d)!=v) {
    dataStore[d]=v;
    emit dataChanged(d, v);
  }
}

void ImageDataStore::addTransform(const QTransform &t) {
  imageTransform = t * imageTransform;
  emit transformChanged();
}

QSizeF ImageDataStore::getTransformedSizeData(DataType d) const {
  if (hasData(d)) {
    return transformSize(getData(d).toSizeF(), imageTransform);
  } else {
    return QSizeF();
  }
}

void ImageDataStore::setTransformedSizeData(DataType d, const QSizeF& s) {
  setData(d, transformSize(s, imageTransform.inverted()));
}
