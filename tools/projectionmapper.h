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

#ifndef PROJECTIONMAPPER_H
#define PROJECTIONMAPPER_H

#include <QObject>
#include <QGraphicsItem>
#include <QMutex>
#include <QFutureWatcher>

#include "core/reflection.h"


class Projector;

class ProjectionMapper: public QObject {
  Q_OBJECT
public:
  ProjectionMapper(Projector* p);
  ProjectionMapper(const ProjectionMapper&);
  ~ProjectionMapper();
  void start();
  void operator()(const Reflection&);
protected slots:
  void finished();
signals:
  void mapFinished(QList<QPointF>, QList<QGraphicsItem*>);
private:
  struct ProjectionMapperPrivate {
    QList<QPointF> coordinates;
    QList<QGraphicsItem*> textItems;
    QAtomicInt ref;
    QFutureWatcher<void> watcher;
    QMutex mutex;
  };
  ProjectionMapperPrivate* d;
  Projector* projector;
  QVector<Reflection> reflections;
  double maxHklSqSum;
  double textSize;
};


#endif // PROJECTIONMAPPER_H
