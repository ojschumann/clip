#include "datascaler.h"
#include <iostream>

#include "image/dataprovider.h"
#include "image/BezierCurve.h"
#include "tools/xmltools.h"

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
  sqareToRaw = t * sqareToRaw;
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

QList<QWidget*> DataScaler::toolboxPages() {
  return QList<QWidget*>();
}


void DataScaler::saveToXML(QDomElement base) {
  QDomElement scaler = ensureElement(base, "Scaler");
  QDomElement t = scaler.appendChild(base.ownerDocument().createElement("Transform")).toElement();
  t.setAttribute("m11", sqareToRaw.m11());
  t.setAttribute("m12", sqareToRaw.m12());
  t.setAttribute("m13", sqareToRaw.m13());
  t.setAttribute("m21", sqareToRaw.m21());
  t.setAttribute("m22", sqareToRaw.m22());
  t.setAttribute("m23", sqareToRaw.m23());
  t.setAttribute("m31", sqareToRaw.m31());
  t.setAttribute("m32", sqareToRaw.m32());
  t.setAttribute("m33", sqareToRaw.m33());
}

void DataScaler::loadFromXML(QDomElement base) {
  QDomElement t = base.firstChildElement("Scaler").firstChildElement("Transform");
  if (t.isElement()) {
    bool ok=true;
    QTransform transform(readDouble(t, "m11", ok),
                         readDouble(t, "m12", ok),
                         readDouble(t, "m13", ok),
                         readDouble(t, "m21", ok),
                         readDouble(t, "m22", ok),
                         readDouble(t, "m23", ok),
                         readDouble(t, "m31", ok),
                         readDouble(t, "m32", ok),
                         readDouble(t, "m33", ok));
    if (ok) {
      sqareToRaw = transform;
      emit imageContentsChanged();
    }
  }
}
