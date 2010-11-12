#include "datascaler.h"
#include <iostream>

#include "image/dataprovider.h"

using namespace std;


DataScaler::DataScaler(DataProvider* dp, QObject *parent) :
    QObject(parent),
    provider(dp), cache(0), sourceRect()
{
  cout << "init DataScaler" << endl;
  QPolygonF poly(QRectF(0,0,provider->width(), provider->height()));
  poly.pop_back();
  QTransform::squareToQuad(poly, sqareToRaw);
}

DataScaler::~DataScaler() {
  cout << "delete DataScaler" << endl;
}

void DataScaler::addTransform(const QTransform & t) {
  sqareToRaw = t* sqareToRaw;
  if (cache)
    redrawCache();
  emit imageContentsChanged();
}

QImage DataScaler::getImage(const QSize &size, const QRectF &_sourceRect) {
  if ((cache==0) || (size!=cache->size()) || (_sourceRect!=sourceRect)) {
    if (cache) delete cache;
    cache = new QImage(size, QImage::Format_ARGB32_Premultiplied);
    sourceRect = _sourceRect;
    redrawCache();
  }
  return *cache;
}
