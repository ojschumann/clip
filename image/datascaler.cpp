#include "datascaler.h"


DataScaler::DataScaler(DataProvider* dp, QObject *parent) :
    QObject(parent),
    provider(dp)
{

}

QImage DataScaler::getImage(const QSize &size, const QRectF &from) {
  if ((size!=cache.size()) || (from!=lastFrom)) {
    redrawCache();
  }
  return cache;
}
