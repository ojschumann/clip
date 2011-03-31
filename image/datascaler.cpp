#include "datascaler.h"
#include <iostream>
#include <cmath>

#include "defs.h"
#include "image/dataprovider.h"
#include "image/BezierCurve.h"
#include "tools/xmltools.h"

using namespace std;


DataScaler::DataScaler(DataProvider* dp, QObject *parent) :
    QObject(parent),
    provider(dp), cache(0), sourceRect()
{
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
}

#include "tools/debug.h"


QTransform DataScaler::initialTransform() {
  QSize s = provider->size();
  return QTransform(s.width(), 0, 0, -s.height(), 0, s.height());
}

void DataScaler::resetAllTransforms() {
  sqareToRaw = initialTransform();
  emit imageContentsChanged();
}

void DataScaler::addTransform(const QTransform & t) {
  sqareToRaw = t * sqareToRaw;
  if (cache)
    redrawCache();
  emit imageContentsChanged();
}

void DataScaler::updateContrastMapping() {
}

QImage DataScaler::getImage(const QSize &size, const QPolygonF &_sourceRect) {
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
  poly2.pop_back();

  QTransform out2sqare;
  QTransform::quadToQuad(poly2, sourceRect, out2sqare);
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

const char XML_DataScaler_Element[] = "Scaler";
const char XML_DataScaler_Transform[] = "Transform";
const char XML_DataScaler_Transform_m11[] = "m11";
const char XML_DataScaler_Transform_m12[] = "m12";
const char XML_DataScaler_Transform_m13[] = "m13";
const char XML_DataScaler_Transform_m21[] = "m21";
const char XML_DataScaler_Transform_m22[] = "m22";
const char XML_DataScaler_Transform_m23[] = "m23";
const char XML_DataScaler_Transform_m31[] = "m31";
const char XML_DataScaler_Transform_m32[] = "m32";
const char XML_DataScaler_Transform_m33[] = "m33";


void DataScaler::saveToXML(QDomElement base) {
  QDomElement scaler = ensureElement(base, XML_DataScaler_Element);
  QDomElement t = scaler.appendChild(base.ownerDocument().createElement(XML_DataScaler_Transform)).toElement();
  t.setAttribute(XML_DataScaler_Transform_m11, sqareToRaw.m11());
  t.setAttribute(XML_DataScaler_Transform_m12, sqareToRaw.m12());
  t.setAttribute(XML_DataScaler_Transform_m13, sqareToRaw.m13());
  t.setAttribute(XML_DataScaler_Transform_m21, sqareToRaw.m21());
  t.setAttribute(XML_DataScaler_Transform_m22, sqareToRaw.m22());
  t.setAttribute(XML_DataScaler_Transform_m23, sqareToRaw.m23());
  t.setAttribute(XML_DataScaler_Transform_m31, sqareToRaw.m31());
  t.setAttribute(XML_DataScaler_Transform_m32, sqareToRaw.m32());
  t.setAttribute(XML_DataScaler_Transform_m33, sqareToRaw.m33());
}

void DataScaler::loadFromXML(QDomElement base) {
  QDomElement element = base.elementsByTagName(XML_DataScaler_Element).at(0).toElement();
  if (element.isNull()) return;
  for (QDomElement e=element.firstChildElement(); !e.isNull(); e=e.nextSiblingElement()) {
    bool ok = true;
    if (e.tagName()==XML_DataScaler_Transform) {
      QTransform transform(readDouble(e, XML_DataScaler_Transform_m11, ok),
                           readDouble(e, XML_DataScaler_Transform_m12, ok),
                           readDouble(e, XML_DataScaler_Transform_m13, ok),
                           readDouble(e, XML_DataScaler_Transform_m21, ok),
                           readDouble(e, XML_DataScaler_Transform_m22, ok),
                           readDouble(e, XML_DataScaler_Transform_m23, ok),
                           readDouble(e, XML_DataScaler_Transform_m31, ok),
                           readDouble(e, XML_DataScaler_Transform_m32, ok),
                           readDouble(e, XML_DataScaler_Transform_m33, ok));
      if (ok) {
        sqareToRaw = transform;
        if (cache)
          redrawCache();
        emit imageContentsChanged();
      }
    }
  }
}
