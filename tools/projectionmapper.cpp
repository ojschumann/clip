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
