#include "projectionmapper.h"

#include <QtConcurrentMap>
#include <QApplication>

#include "core/projector.h"
#include "core/crystal.h"

ProjectionMapper::ProjectionMapper(Projector *p):
    projector(p)
{
  d = new ProjectionMapperPrivate;
  projector = p;
  d->ref=0;
  d->ref.ref();
  connect(&d->watcher, SIGNAL(finished()), this, SLOT(finished()));
}

ProjectionMapper::~ProjectionMapper() {
  if (!d->ref.deref()) {
    delete d;
    d=0;
  }
}

ProjectionMapper::ProjectionMapper(const ProjectionMapper &o) {
  o.d->ref.ref();
  d = o.d;
  maxHklSqSum = o.maxHklSqSum;
  textSize = o.textSize;
  reflections = o.reflections;
  projector = o.projector;
}

void ProjectionMapper::start() {
  if (!d->watcher.isRunning()) {
    maxHklSqSum = projector->getMaxHklSqSum();
    textSize = projector->getTextSize();
    reflections = projector->getCrystal()->getReflectionList();
    d->coordinates.clear();
    d->textItems.clear();
    d->watcher.setFuture(QtConcurrent::map(reflections.constBegin(), reflections.constEnd(), *this));
  } else {
    // FIXME Do something else
    cout << "ProjectionMapping still running" << endl;
  }
}

void ProjectionMapper::operator ()(const Reflection& r) {
  QPointF p;
  if (projector->project(r, p)) {
    QGraphicsTextItem* t = 0;
    if (r.hklSqSum<=maxHklSqSum) {
      t = new QGraphicsTextItem();
      t->setTransform(QTransform(1,0,0,-1,0,0));
      t->setHtml(r.toHtml());
      t->setPos(p);
      QRectF r=t->boundingRect();
      double s=textSize/std::min(r.width(), r.height());
      t->scale(s,s);
      t->moveToThread(qApp->thread());
    }
    d->mutex.lock();
    d->coordinates << p;
    if (t) d->textItems << t;
    d->mutex.unlock();
  }
}

void ProjectionMapper::finished() {
  QList<QPointF> coo(d->coordinates);
  QList<QGraphicsItem*> text(d->textItems);
  d->coordinates.clear();
  d->textItems.clear();
  emit mapFinished(coo, text);
}
