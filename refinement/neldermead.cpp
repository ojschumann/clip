#include "neldermead.h"
#include "neldermead_worker.h"

#include <QtConcurrentRun>
#include <QMetaType>
#include <iostream>
#include <QTime>

#include "core/crystal.h"
#include "core/projector.h"
#include "refinement/fitparameter.h"
#include "refinement/fitparametergroup.h"

using namespace std;

NelderMead::NelderMead(Crystal* c, QObject *parent) :
    QObject(parent),
    liveCrystal(c),
    shouldStop(false)
{
  connect(&threadWatcher, SIGNAL(finished()), this, SIGNAL(finished()));
  connect(this, SIGNAL(bestSolution(QList<double>)), this, SLOT(setBestSolutionToLiveCrystal(QList<double>)), Qt::QueuedConnection);
}


void NelderMead::start() {
  if (threadWatcher.isRunning()) return;
  shouldStop = false;
  threadWatcher.setFuture(QtConcurrent::run(this, &NelderMead::run));
}

void NelderMead::stop() {
  threadLock.lockForWrite();
  shouldStop = true;
  threadLock.unlock();
  threadWatcher.waitForFinished();
}

bool NelderMead::isRunning() {
  return threadWatcher.isRunning();
}

void NelderMead::run() {
  NMWorker* worker = new NMWorker(liveCrystal);
  int loops = 0;
  int noImprovmentLoops=0;
  QTime rateLimiter;
  double bestEmitedScore=worker->bestScore();
  rateLimiter.start();
  forever {
    loops++;
    noImprovmentLoops++;
    threadLock.lockForRead();
    bool _shouldStop = shouldStop;
    threadLock.unlock();
    if (_shouldStop) {
      break;
    }

    double lastScore = worker->bestScore();
    worker->doOneIteration();
    if (worker->bestScore()<lastScore) {
      noImprovmentLoops = 0;
    }
    if (worker->bestScore()<bestEmitedScore && rateLimiter.elapsed()>100) {
      bestEmitedScore = worker->bestScore();
      emit bestSolutionScore(worker->bestScore());
      emit bestSolution(worker->bestSolution());
      rateLimiter.restart();
    }
    if (noImprovmentLoops>50) {
      break;
    }
    //TODO: For debugging
    QList<double> deltas = worker->parameterRelativeDelta();
    qSort(deltas);
    cout << worker->bestScore() << " " << deltas.first() << " " << deltas.last() << endl;

    QList<FitParameter*> XXp;
    foreach (FitObject* o, liveCrystal->getFitObjects())
      XXp += o->allParameters();
    cout << "  FitValues:";
    foreach (FitParameter* p, XXp) cout << " " << p->value();
    cout << endl;

  }
  emit bestSolutionScore(worker->bestScore());
  emit bestSolution(worker->bestSolution());
  delete worker;
}

void NelderMead::setBestSolutionToLiveCrystal(QList<double> solution) {
  QList<FitParameter*> parameters;
  foreach (FitObject* o, liveCrystal->getFitObjects())
    parameters += o->allParameters();

  if (parameters.size()==solution.size()) {
    for (int n=0; n<parameters.size(); n++) {
      parameters.at(n)->prepareValue(solution.at(n));
    }
    foreach (FitParameter* p, parameters) p->setValue();
    cout << "LocalValues:";
    foreach (FitParameter* p, parameters) cout << " " << p->value();
    cout << endl;
  }
  QList<FitParameter*> XXp;
  foreach (FitObject* o, liveCrystal->getFitObjects())
    XXp += o->allParameters();
  cout << " LiveValues:";
  foreach (FitParameter* p, XXp) cout << " " << p->value();
  cout << endl;


}


int QListDoubleRegistered = qRegisterMetaType< QList<double> >();
