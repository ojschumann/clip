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

#include "spotindicatorgraphicsitem.h"

#include <QPainter>
#include <QGraphicsView>
#include <cmath>
#include <QDebug>

#include "config/configstore.h"
#include "threadrunner.h"

using namespace std;




SpotIndicatorGraphicsItem::SpotIndicatorGraphicsItem():
    QGraphicsObject(),
    tWorker(this),
    threadRunner(new ThreadRunner(tWorker))
{
  ConfigStore::getInstance()->ensureColor(ConfigStore::SpotIndicators, this, SLOT(setColor(QColor)));
  setCacheMode(NoCache);
  cacheNeedsUpdate = true;
  setCachedPainting();
  cache = nullptr;
};

SpotIndicatorGraphicsItem::~SpotIndicatorGraphicsItem() {
  delete threadRunner;
  delete cache;
}

void SpotIndicatorGraphicsItem::setColor(QColor c) {
  spotColor = c;
  cacheNeedsUpdate = true;
  update();
}


#include "defs.h"

void SpotIndicatorGraphicsItem::updateCache() {
  if (cacheNeedsUpdate) {
    //qDebug() << "SpotIndicatorGraphicsItem::updateCache()";

    threadRunner->start();
    cache->fill(QColor(0,0,0,0));
    threadRunner->join();

    cacheNeedsUpdate=false;
  }
}

void SpotIndicatorGraphicsItem::paint(QPainter *p, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
  if (cachedPainting) {
    if (!cache || (cache->size()!=p->viewport().size())) {
      if (cache) delete cache;
      cache = new QPixmap(p->viewport().size());
      cacheNeedsUpdate = true;
    }

    if (transform!=p->worldTransform()) {
      transform = p->worldTransform();
      cacheNeedsUpdate = true;
    }

    updateCache();

    p->save();
    p->resetTransform();
    p->drawPixmap(QPoint(0,0), *cache);
    p->restore();
  } else {
    //PDF-Export via QPrinter::setOutputFormat(PdfFormat) has a Bug concerning
    //Cosmetic Pens and very small coordinates (here, the rect is (0, 0, 1, 1))
    //thus reset the World Transform and paint with noncosmetic pens
    //
    //SVG-Export via QSvgGenerator doesn't obey the QPainter-Clip, thus do some clipping here

    QPen pen(spotColor);
    pen.setWidthF(1.0);
    pen.setCosmetic(false);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    QTransform t = p->worldTransform();
    p->resetTransform();

    QPainterPath circle;
    circle.addEllipse(QPointF(0, 0), spotSize*t.m11(), spotSize*t.m22());

    for (int i=0; i<coordinates.size(); i++) {
      QPainterPath movedCircle = circle.translated(t.map(coordinates.at(i)));
      if (movedCircle.intersects(p->clipPath()))
        p->drawPath(movedCircle);
    }
    p->setTransform(t);
  }
}

void SpotIndicatorGraphicsItem::pointsUpdated() {
  cacheNeedsUpdate = true;
  prepareGeometryChange();
}

QRectF SpotIndicatorGraphicsItem::boundingRect() const {
  if (coordinates.size()>0) {
    QRectF r(coordinates.at(0), QSizeF(0,0));
    for (int i=1; i<coordinates.size(); i++) {
      QPointF p(coordinates.at(i));
      r.setLeft(qMin(p.x(), r.left()));
      r.setRight(qMax(p.x(), r.right()));
      r.setTop(qMin(p.y(), r.top()));
      r.setBottom(qMax(p.y(), r.bottom()));
    }
    return r;
  } else {
    return QRectF();
  }
}


SpotIndicatorGraphicsItem::TWorker::TWorker(SpotIndicatorGraphicsItem *s):
    wp(0),
    spotIndicator(s)
  {}

SpotIndicatorGraphicsItem::TWorker::~TWorker() {
  init(0);
}

void SpotIndicatorGraphicsItem::TWorker::init(int numberOfThreads) {
  if (threadCaches.size()!=numberOfThreads) {
    foreach (CacheType* c, threadCaches)
      delete c;
    threadCaches.clear();
    for (int n=0; n<numberOfThreads; n++)
      threadCaches.push_back(nullptr);
  }

  wp = 0;
}

void SpotIndicatorGraphicsItem::TWorker::operator ()(int id) {
  CacheType* localCache = threadCaches[id];
  if (localCache==nullptr || localCache->size()!=spotIndicator->cache->size()) {
    if (localCache!=nullptr) delete localCache;
    localCache = new CacheType(spotIndicator->cache->size(), QImage::Format_ARGB32_Premultiplied);
    threadCaches[id] = localCache;
  }

  double rx = spotIndicator->transform.m11()*spotIndicator->spotSize;
  double ry = spotIndicator->transform.m22()*spotIndicator->spotSize;
  localCache->fill(QColor(0,0,0,0).rgba());
  //localCache->fill(QColor(0,0,0,0));

  QPainter painter(localCache);
  QList<QGraphicsView*> l = spotIndicator->scene()->views();
  if (l.size())
    painter.setRenderHints(l.at(0)->renderHints());
  painter.setPen(spotIndicator->spotColor);

  int size = spotIndicator->coordinates.size();
  int chunkSize = std::max(size/static_cast<int>(threadCaches.size()), 1);
  QTransform t = spotIndicator->transform;

  int n=id;
  //while ((n=wp.fetchAndAddOrdered(1))*chunkSize<size) {
    QPointF const* p = spotIndicator->coordinates.constData()+n*chunkSize;
    int count = std::min(size, chunkSize*(n+1))-chunkSize*n;
    for (int i=0; i<count; i++) {
      painter.drawEllipse(t.map(*(p++)), rx, ry);
    }
  //}

  painter.end();

}

void SpotIndicatorGraphicsItem::TWorker::done() {
  QPainter painter(spotIndicator->cache);
  foreach (CacheType* c, threadCaches) {
    painter.drawImage(QPoint(0, 0), *c);
    //painter.drawPixmap(0, 0, *c);
  }
  painter.end();
}

