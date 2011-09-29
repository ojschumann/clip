/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

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
 
#include <QDebug>

#include "config/configstore.h"

using namespace std;

#include <cmath>

struct Mean {
  Mean(): N(0), sum(0), sumSq(0) {};

  void add(double value) { N++; sum += value; sumSq += value*value; }
  double mean() { return sum/N; }
  double var() { return sqrt(sumSq/N - sum/N*sum/N); }
  int N;
  double sum;
  double sumSq;
};

SpotIndicatorGraphicsItem::SpotIndicatorGraphicsItem():
    QGraphicsObject(),
    workerPermission(0),
    workerSync(0),
    tWorker(this),
    threadRunner(tWorker)
{
  ConfigStore::getInstance()->ensureColor(ConfigStore::SpotIndicators, this, SLOT(setColor(QColor)));
  setCacheMode(NoCache);
  cacheNeedsUpdate = true;
  setCachedPainting();
  cache = nullptr;
  for (int i=0; i<QThread::idealThreadCount(); i++) {
    Worker* w = new Worker(this, i);
    w->start();
    workers << w;
  }

  startTimes.resize(workers.size());
  stopTimes.resize(workers.size());
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


#include "defs.h"

void SpotIndicatorGraphicsItem::updateCache() {
  if (cacheNeedsUpdate) {

    static Mean QThreadTime;
    static Mean TWorkerTime;
    static Mean relative;

    unsigned long long t1 = rdtsctime();

    static Mean runtimes;

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
    p.end();


    unsigned long long dtstart = (startTimes[0] > startTimes[1]) ? startTimes[0] - startTimes[1] : startTimes[1] - startTimes[0];
    unsigned long long dtstop = (stopTimes[0] > stopTimes[1]) ? stopTimes[0] - stopTimes[1] : stopTimes[1] - stopTimes[0];

    runtimes.add(stopTimes[0]-startTimes[0]);
    runtimes.add(stopTimes[1]-startTimes[1]);

    qDebug() << "QThread delta" << startTimes.size() << dtstart << dtstop << stopTimes[0]-startTimes[0] << stopTimes[1]-startTimes[1] << runtimes.mean() << runtimes.var();
    unsigned long long t2 = rdtsctime();

    threadRunner.start();
    cache->fill(QColor(0,0,0,0));
    threadRunner.join();
    unsigned long long t3 = rdtsctime();

    double s = 1.0*(t2-t1)/(t3-t2);
    QThreadTime.add(t2-t1);
    TWorkerTime.add(t3-t2);
    relative.add(s);

    double delta = std::abs(TWorkerTime.mean()-QThreadTime.mean());
    double ddelta = sqrt(TWorkerTime.var()*TWorkerTime.var()+QThreadTime.var()*QThreadTime.var());

    qDebug() << "TP" << QThreadTime.mean() << QThreadTime.var() << TWorkerTime.mean() << TWorkerTime.var() << relative.mean() << relative.var() << delta << ddelta;

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

    if (!localCache || localCache->size()!=spotIndicator->cache->size()) {
      if (localCache) delete localCache;
      localCache = new QImage(spotIndicator->cache->size(), QImage::Format_ARGB32_Premultiplied);
    }
    double rx = spotIndicator->transform.m11()*spotIndicator->spotSize;
    double ry = spotIndicator->transform.m22()*spotIndicator->spotSize;
    localCache->fill(QColor(0,0,0,0).rgba());
    QPainter painter(localCache);
    QList<QGraphicsView*> l = spotIndicator->scene()->views();
    if (l.size())
      painter.setRenderHints(l.at(0)->renderHints());
    painter.setPen(spotIndicator->spotColor);
    int i;
    int maxCoo = spotIndicator->coordinates.size();
    unsigned long long tstart = rdtsctime();
    while ((i=spotIndicator->workN.fetchAndAddOrdered(1))<maxCoo) {
      painter.drawEllipse(spotIndicator->transform.map(spotIndicator->coordinates.at(i)), rx, ry);
    }
    unsigned long long tstop = rdtsctime();
    painter.end();

    spotIndicator->startTimes[threadNr] = tstart;
    spotIndicator->stopTimes[threadNr] = tstop;

    spotIndicator->workerSync.release();

  }

}


SpotIndicatorGraphicsItem::TWorker::TWorker(SpotIndicatorGraphicsItem *s):
    wp(0),
    spotIndicator(s),
    runtimes(0),
    runs(0)
  {}

SpotIndicatorGraphicsItem::TWorker::~TWorker() {
  init(0);
}

void SpotIndicatorGraphicsItem::TWorker::init(int numberOfThreads) {
  if (threadCaches.size()!=numberOfThreads) {
    for (CacheType* c: threadCaches)
      delete c;
    threadCaches.clear();
    for (int n=0; n<numberOfThreads; n++)
      threadCaches.push_back(nullptr);
  }

  startTimes.resize(numberOfThreads);
  stopTimes.resize(numberOfThreads);

  wp = 0;
  spotIndicator->workN = 0;
}

void SpotIndicatorGraphicsItem::TWorker::operator ()(int id) {


  CacheType* localCache = threadCaches[id];
  if (localCache==nullptr || localCache->size()!=spotIndicator->cache->size()) {
    if (localCache==nullptr) delete localCache;
    localCache = new CacheType(spotIndicator->cache->size(), QImage::Format_ARGB32_Premultiplied);
    qDebug() << "new cache";
    //localCache = new CacheType(spotIndicator->cache->size());
    threadCaches[id] = localCache;
  }

  int size = spotIndicator->coordinates.size();
  int chunkSize = std::max(size/4, 1);

  localCache->fill(QColor(0,0,0,0).rgba());
  //localCache->fill(QColor(0,0,0,0));

  QTransform t = spotIndicator->transform;
  double rx = t.m11()*spotIndicator->spotSize;
  double ry = t.m22()*spotIndicator->spotSize;

  QPainter painter(localCache);
  QList<QGraphicsView*> l = spotIndicator->scene()->views();
  if (l.size())
    painter.setRenderHints(l.at(0)->renderHints());
  painter.setPen(spotIndicator->spotColor);

  int i;
/*  while ((n=wp.fetchAndAddOrdered(1))*chunkSize<size) {
    QPointF const* p = spotIndicator->coordinates.constData()+n*chunkSize;
    int count = std::min(size, chunkSize*(n+1))-chunkSize*n;
    for (int i=0; i<count; i++) {
      painter.drawEllipse(t.map(*(p++)), rx, ry);
    }
  }*/

  unsigned long long tstart = rdtsctime();
  while ((i=spotIndicator->workN.fetchAndAddOrdered(1))<size) {
    painter.drawEllipse(spotIndicator->transform.map(spotIndicator->coordinates.at(i)), rx, ry);
  }
  unsigned long long tstop = rdtsctime();

  painter.end();

  startTimes[id] = tstart;
  stopTimes[id] = tstop;
}

void SpotIndicatorGraphicsItem::TWorker::done(int numberOfThreads) {
  QPainter painter(spotIndicator->cache);
  for (CacheType* c: threadCaches) {
    painter.drawImage(0, 0, *c);
    //painter.drawPixmap(0, 0, *c);
  }
  painter.end();

  unsigned long long dtstart = (startTimes[0] > startTimes[1]) ? startTimes[0] - startTimes[1] : startTimes[1] - startTimes[0];
  unsigned long long dtstop = (stopTimes[0] > stopTimes[1]) ? stopTimes[0] - stopTimes[1] : stopTimes[1] - stopTimes[0];
  runtimes += stopTimes[0]-startTimes[0] + stopTimes[1]-startTimes[1];
  runs += 2;

  qDebug() << "TWorker delta" << startTimes.size() << dtstart << dtstop << stopTimes[0]-startTimes[0] << stopTimes[1]-startTimes[1] << runtimes/runs;


}

