#include "neldermead.h"
#include "neldermead_worker.h"

#include <QtConcurrentRun>
#include <QMetaType>
#include <iostream>
#include <QTime>
#include <algorithm>

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
  connect(this, SIGNAL(bestSolution(QList<double>, QList<double>)), this, SLOT(setBestSolutionToLiveCrystal(QList<double>, QList<double>)), Qt::QueuedConnection);
}

NelderMead::~NelderMead() {
  stop();
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
    if (worker->bestScore()<bestEmitedScore && rateLimiter.elapsed()>50) {
      bestEmitedScore = worker->bestScore();
      emit bestSolutionScore(worker->bestScore());
      emit bestSolution(worker->bestSolution(), worker->calcDeviation());
      rateLimiter.restart();
    }
    if (lastScore/worker->bestScore()>1.001) {
      noImprovmentLoops = 0;
    }
    double relativeScoreDifference = (worker->worstScore() - worker->bestScore())/worker->worstScore();
    if ((noImprovmentLoops>75) || (relativeScoreDifference < 1e-4)) {
      break;
    }
  }
  emit bestSolutionScore(worker->bestScore());
  emit bestSolution(worker->bestSolution(), worker->calcDeviation());
  delete worker;
}

void NelderMead::setBestSolutionToLiveCrystal(QList<double> solution, QList<double> deviation) {
  QList<FitParameter*> parameters;
  foreach (FitObject* o, liveCrystal->getFitObjects())
    parameters += o->enabledParameters();

  if (parameters.size()==solution.size()) {
    for (int n=0; n<parameters.size(); n++) parameters.at(n)->prepareValue(solution.at(n));
    for (int n=0; n<parameters.size(); n++) parameters.at(n)->setValue(deviation.at(n));
  }
}


int QListDoubleRegistered = qRegisterMetaType< QList<double> >();
