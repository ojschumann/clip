#include "spotindicatorgraphicsitem.h"

#include <QPainter>
#include <QGraphicsView>
#include <iostream>

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
      cout << "Worker " << i << " still running since " << loop << " loops" << endl;
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
      if (worker->localCache)
        p.drawImage(QPoint(0,0), *worker->localCache);
    }
    cacheNeedsUpdate=false;
  }
}

void SpotIndicatorGraphicsItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w) {
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
    p->setPen(spotColor);
    for (int i=0; i<coordinates.size(); i++) {
      p->drawEllipse(coordinates.at(i), spotSize, spotSize);
    }
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

