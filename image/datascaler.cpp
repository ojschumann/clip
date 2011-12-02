/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

#include "datascaler.h"
 
#include <cmath>

#include "image/dataprovider.h"
#include "image/beziercurve.h"
#include "tools/xmltools.h"
#include "tools/threadrunner.h"
using namespace std;


DataScaler::DataScaler(DataProvider* dp, QObject* _parent) :
    QObject(_parent),
    provider(dp), cache(nullptr), sourceRect(), threads(new ThreadRunner())
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
  delete threads;
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
  redrawCache(); // redrawCache checks for (cache==nullptr) by itself, so it is safe

  emit imageContentsChanged();
}

void DataScaler::updateContrastMapping() {
}

QImage DataScaler::getImage(const QSize &size, const QPolygonF &_sourceRect) {
  if ((cache==nullptr) || (size!=cache->size()) || (_sourceRect!=sourceRect)) {
    if (cache!=nullptr) delete cache;
    cache = new QImage(size, QImage::Format_ARGB32_Premultiplied);
    sourceRect = _sourceRect;
    redrawCache();
  }
  return *cache;
}

DataScaler::Mapper::Mapper(DataScaler* s): scaler(s) {}

void DataScaler::Mapper::init() {
  line = 0;

  QPolygonF poly2(QRectF(0,0,scaler->cache->width(),scaler->cache->height()));
  poly2.pop_back();

  QTransform out2sqare;
  QTransform::quadToQuad(poly2, scaler->sourceRect, out2sqare);
  t = QTransform::fromTranslate(0.5, 0.5) * out2sqare * QTransform(1,0,0,-1,0,1) * scaler->sqareToRaw;

}

void DataScaler::Mapper::operator()(int threadId) {
  int w = scaler->cache->width();
  int h = scaler->cache->height();
  int y;
  QRgb* data = reinterpret_cast<QRgb*>(scaler->cache->bits());

  QPointF dx = t.map(QPointF(1, 0))-t.map(QPointF(0, 0));
  while ((y = line.fetchAndAddOrdered(1))<h) {
    QRgb* d = data+y*w;
    QPointF p = t.map(QPointF(0, y));
    for (int x=0; x<w; x++) {
      *(d++) = scaler->getRGB(p);
      p+=dx;
    }
  }
}


void DataScaler::redrawCache() {
  if (cache==nullptr) return;

  threads->start(Mapper(this));
  threads->join();
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
        if (cache!=nullptr)
          redrawCache();
        emit imageContentsChanged();
      }
    }
  }
}
