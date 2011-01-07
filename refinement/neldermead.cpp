#include "neldermead.h"
#include "neldermead_worker.h"

#include <QtConcurrentRun>
#include <QMetaType>
#include <iostream>
#include <QTimer>

#include "core/crystal.h"
#include "core/projector.h"

using namespace std;

NelderMead::NelderMead(Crystal* c, QObject *parent) :
    QObject(parent),
    liveCrystal(c),
    shouldStop(false)
{
  connect(&threadWatcher, SIGNAL(finished()), this, SLOT(publishBestSolution()));
  connect(&publishTimer, SIGNAL(timeout()), this, SLOT(publishBestSolution()));

}


void NelderMead::start() {
  if (threadWatcher.isRunning()) return;
  shouldStop = false;
  threadWatcher.setFuture(QtConcurrent::run(this, &NelderMead::run));
  publishTimer.start(200);
}

void NelderMead::stop() {
  threadLock.lockForWrite();
  shouldStop = true;
  threadLock.unlock();
  threadWatcher.waitForFinished();
}
void NelderMead::run() {
  NMWorker* worker = new NMWorker(liveCrystal);
  threadLock.lockForWrite();
  bestScore = worker->bestScore();
  threadLock.unlock();
  int loops = 0;
  int noImprovmentLoops=0;
  forever {
    loops++;
    noImprovmentLoops++;
    threadLock.lockForRead();
    bool _shouldStop = shouldStop;
    threadLock.unlock();
    if (_shouldStop) {
      delete worker;
      return;
    }

    double lastScore = worker->bestScore();
    worker->doOneIteration();
    if (lastScore>worker->bestScore()) {
      noImprovmentLoops = 0;
      threadLock.lockForWrite();
      bestSolution = worker->bestSolution();
      bestSolutionAlreadyPublished = false;
      bestScore = worker->bestScore();
      threadLock.unlock();
    }
    if (noImprovmentLoops>5000) {
      noImprovmentLoops = 0;
      cout << "RestartWorker" << endl;
      worker->restart();
    }
  }
  threadLock.lockForWrite();
  bestSolution = worker->bestSolution();
  threadLock.unlock();
  delete worker;
}

void NelderMead::publishBestSolution() {
  if (!threadWatcher.isRunning()) publishTimer.stop();
  threadLock.lockForRead();
  QList<double> localBestSolution = bestSolution;
  double localBestScore = bestScore;
  bool _bestSolutionAlreadyPublished = bestSolutionAlreadyPublished;
  threadLock.unlock();

  if (!_bestSolutionAlreadyPublished) {
    cout << "Best Solution " << localBestScore << " Delta " << lastBestScore-localBestScore;
    lastBestScore = localBestScore;
    threadLock.lockForWrite();
    bestSolutionAlreadyPublished = true;
    threadLock.unlock();

    QList<FitParameter*> parameters;
    parameters += liveCrystal->enabledParameters();
    foreach (Projector* p, liveCrystal->getConnectedProjectors())
      if (p->hasMarkers())
        parameters += p->enabledParameters();

    if (parameters.size()==localBestSolution.size()) {
      for (int n=0; n<parameters.size(); n++) {
        parameters.at(n)->prepareValue(localBestSolution.at(n));
        cout << " " << localBestSolution.at(n);
      }
      cout << endl;
      foreach (FitParameter* p, parameters) p->setValue();
    }


  }
}





int QListDoubleRegistered = qRegisterMetaType< QList<double> >();
