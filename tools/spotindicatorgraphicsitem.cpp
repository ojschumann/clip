/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

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
#include <iostream>
#include <QDebug>

#include "config/configstore.h"

using namespace std;

SpotIndicatorGraphicsItem::SpotIndicatorGraphicsItem():
    QGraphicsObject(),
    workerPermission(0),
    workerSync(0)
{
  ConfigStore::getInstance()->ensureColor(ConfigStore::SpotIndicators, this, SLOT(setColor(QColor)));
  setCacheMode(NoCache);
  cacheNeedsUpdate = true;
  setCachedPainting();
  cache = 0;
  for (int i=0; i<QThread::idealThreadCount(); i++) {
    Worker* w = new Worker(this, i);
    w->start();
    workers << w;
  }
};

SpotIndicatorGraphicsItem::~SpotIndicatorGraphicsItem() {
  for (int i=0; i<workers.size(); i++) {
    workers.at(i)->shouldStop=true;
  }
  workerPermission.release(workers.size());
  workerSync.acquire(workers.size());
  for (int i=0; i<workers.size(); i++) {
    for (int loop=0; !workers[i]->wait(250); loop++) {
      qDebug() << "Worker" << i << "still running since" << loop << "loops";
      if (loop>5) workers[i]->terminate();
    }
    delete workers[i];
  }
  workers.clear();
  delete cache;
}

void SpotIndicatorGraphicsItem::setColor(QColor c) {
  spotColor = c;
  cacheNeedsUpdate = true;
  update();
}

void SpotIndicatorGraphicsItem::updateCache() {
  if (cacheNeedsUpdate) {
    workN = 0;
    workerPermission.release(workers.size());
    cache->fill(QColor(0,0,0,0));
    workerSync.acquire(workers.size());

    QPainter p(cache);
    foreach (Worker* worker, workers) {
      if (worker->localCache) {
        p.drawImage(QPoint(0,0), *worker->localCache);
      } else {
        qDebug() << "No chache for Worker:" << workers.indexOf(worker);
      }
    }
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
      //p->drawEllipse(t.map(coordinates.at(i)), spotSize*t.m11(), spotSize*t.m22());
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


void SpotIndicatorGraphicsItem::Worker::run() {
  forever {
    spotIndicator->workerPermission.acquire();
    if (shouldStop) {
      spotIndicator->workerSync.release(1);
      if (!localCache)
        delete localCache;
      return;
    }

    double rx = spotIndicator->transform.m11()*spotIndicator->spotSize;
    double ry = spotIndicator->transform.m22()*spotIndicator->spotSize;


    if (!localCache || localCache->size()!=spotIndicator->cache->size()) {
      if (localCache) delete localCache;
      localCache = new QImage(spotIndicator->cache->size(), QImage::Format_ARGB32_Premultiplied);
    }
    localCache->fill(QColor(0,0,0,0).rgba());
    QPainter painter(localCache);
    QList<QGraphicsView*> l = spotIndicator->scene()->views();
    if (l.size())
      painter.setRenderHints(l.at(0)->renderHints());
    painter.setPen(spotIndicator->spotColor);
    int i;
    int maxCoo = spotIndicator->coordinates.size();
    while ((i=spotIndicator->workN.fetchAndAddOrdered(1))<maxCoo) {
      painter.drawEllipse(spotIndicator->transform.map(spotIndicator->coordinates.at(i)), rx, ry);

    }
    painter.end();
    spotIndicator->workerSync.release();

  }

}

