#include "datascaler.h"
#include <iostream>

using namespace std;


DataScaler::DataScaler(DataProvider* dp, QObject *parent) :
    QObject(parent),
    provider(dp), cache(0), sourceRect()
{
  cout << "init DataScaler" << endl;
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
