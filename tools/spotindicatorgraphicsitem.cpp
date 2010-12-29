#include "spotindicatorgraphicsitem.h"

#include <QPainter>
#include <QGraphicsView>
#include <iostream>

using namespace std;

SpotIndicatorGraphicsItem::SpotIndicatorGraphicsItem(): QGraphicsItem(), workerSync(0) {
  setCacheMode(NoCache);
  cacheNeedsUpdate = true;
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
  workerStart.wakeAll();
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

void SpotIndicatorGraphicsItem::updateCache() {
  if (cacheNeedsUpdate) {
    workN = 0;
    workerStart.wakeAll();
    cache->fill(QColor(0,0,0,0));
    workerSync.acquire(workers.size());

    QPainter p2(cache);
    foreach (Worker* worker, workers) {
      p2.drawImage(QPoint(0,0), *worker->localCache);
    }
    cacheNeedsUpdate=false;
  }
}

void SpotIndicatorGraphicsItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w) {

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
    spotIndicator->mutex.lock();
    spotIndicator->workerStart.wait(&spotIndicator->mutex);
    spotIndicator->mutex.unlock();

    if (shouldStop) {
      spotIndicator->workerSync.release(1);
      if (!localCache)
        delete localCache;
      return;
    }

    double rx = spotIndicator->transform.m11()*spotIndicator->spotSize;
    double ry = spotIndicator->transform.m22()*spotIndicator->spotSize;


    if (!localCache || localCache->size()!=spotIndicator->cache->size())
      localCache = new QImage(spotIndicator->cache->size(), QImage::Format_ARGB32_Premultiplied);
    localCache->fill(QColor(0,0,0,0).rgba());
    QPainter painter(localCache);
    QList<QGraphicsView*> l = spotIndicator->scene()->views();
    if (l.size())
      painter.setRenderHints(l.at(0)->renderHints());
    painter.setPen(Qt::green);
    int i;
    int maxCoo = spotIndicator->coordinates.size();
    while ((i=spotIndicator->workN.fetchAndAddOrdered(1))<maxCoo) {
      painter.drawEllipse(spotIndicator->transform.map(spotIndicator->coordinates.at(i)), rx, ry);

    }
    painter.end();
    spotIndicator->workerSync.release(1);

  }

}

