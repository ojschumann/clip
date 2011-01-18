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
