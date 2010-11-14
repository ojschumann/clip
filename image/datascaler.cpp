#include "datascaler.h"
#include <iostream>

#include "image/dataprovider.h"
#include "image/BezierCurve.h"

using namespace std;


DataScaler::DataScaler(DataProvider* dp, QObject *parent) :
    QObject(parent),
    provider(dp), cache(0), sourceRect()
{
  cout << "init DataScaler" << endl;
  for (int n=0; n<4; n++) {
    BezierCurve* curve = new BezierCurve();
    transferCurves << curve;
    connect(curve, SIGNAL(curveChanged()), this, SLOT(updateContrastMapping()));
  }
  resetAllTransforms();
}

DataScaler::~DataScaler() {
  for (int n=0; n<4; n++)
    delete transferCurves[n];
  transferCurves.clear();
  cout << "delete DataScaler" << endl;
}

#include "tools/debug.h"

void DataScaler::addTransform(const QTransform & t) {
  QList<QPointF> l;
  l << QPointF(0,0) << QPointF(0,1) << QPointF(1,1) << QPointF(1,0) << QPointF(0.5,0.5);
  foreach (QPointF p, l) {
    printPoint("Before", sqareToRaw.map(p));
  }
  sqareToRaw = t * sqareToRaw;
  foreach (QPointF p, l) {
    printPoint("After", sqareToRaw.map(p));
  }
  if (cache)
    redrawCache();
  emit imageContentsChanged();
}

void DataScaler::resetAllTransforms() {
  QPolygonF poly(QRectF(0,0,provider->width(), provider->height()));
  poly.pop_back();
  QTransform::squareToQuad(poly, sqareToRaw);
  QTransform flipy (1,0,0,-1,0,1);
  sqareToRaw = flipy * sqareToRaw;
  emit imageContentsChanged();
}

void DataScaler::updateContrastMapping() {
  cout << "DataScaler::updateContrastMapping()" << endl;
}

QImage DataScaler::getImage(const QSize &size, const QRectF &_sourceRect) {
  if ((cache==0) || (size!=cache->size()) || (_sourceRect!=sourceRect)) {
    if (cache) delete cache;
    cache = new QImage(size, QImage::Format_ARGB32_Premultiplied);
    sourceRect = _sourceRect;
    redrawCache();
  }
  cout << "getImage" << endl;
  return *cache;
}

void DataScaler::redrawCache() {
  if (!cache) return;

  QRgb* data = (QRgb*)cache->bits();

  int w = cache->width();
  int h = cache->height();


  QPolygonF poly2(QRectF(0,0,w,h));
  QPolygonF poly3(sourceRect);
  poly2.pop_back();
  poly3.pop_back();

  QTransform out2sqare;
  QTransform::quadToQuad(poly2, poly3, out2sqare);
  QTransform t = QTransform::fromTranslate(0.5, 0.5) * out2sqare * QTransform(1,0,0,-1,0,1) * sqareToRaw;

  // TODO: Multithreaded implementation
  for (int y=0; y<h; y++) {
    for (int x=0; x<w; x++) {
      *data = getRGB(t.map(QPointF(x, y)));
      data++;
    }
  }
}

