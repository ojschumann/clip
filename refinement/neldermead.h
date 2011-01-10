#ifndef NELDERMEAD_H
#define NELDERMEAD_H

#include <QObject>
#include <QFutureWatcher>
#include <QMetaType>
#include <QReadWriteLock>
#include <QTimer>

#include "tools/vec3D.h"
#include "tools/mat3D.h"

class FitObject;
class FitParameter;
class Crystal;
class AbstractMarkerItem;

class NelderMead : public QObject
{
  Q_OBJECT
public:
  explicit NelderMead(Crystal* c, QObject *parent = 0);
  bool isRunning();
public slots:
  void start();
  void stop();
signals:
  void finished();
  void bestSolutionScore(double);
protected slots:
  void publishBestSolution();
protected:
  class Worker;
  void run();


  // Crystal, that is used in the UI
  Crystal* liveCrystal;

  QTimer publishTimer;

  QFutureWatcher<void> threadWatcher;
  QReadWriteLock threadLock;
  bool shouldStop;
  QList<double> bestSolution;
  double bestScore;
  double lastBestScore;
  bool bestSolutionAlreadyPublished;

};

Q_DECLARE_METATYPE(QList<double>)
#endif // NELDERMEAD_H
