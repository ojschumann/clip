/**************************************************************************
  Copyright (C) 2011 schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

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
**************************************************************************/

#ifndef THREADRUNNER_H
#define THREADRUNNER_H

#include <vector>

#include "config.h"

#define USE_QTHREADS 1
#define USE_SEMAPHORE_SYNC 1

#if USE_QTHREADS
#include <QThread>
#else
#include <thread>
#include <boost/thread.hpp>
#endif

#if USE_SEMAPHORE_SYNC
#include <QSemaphore>
#endif

class ThreadRunner {
public:
  template <class WORKER> ThreadRunner(WORKER&& w):
#if USE_SEMAPHORE_SYNC
      workerPermission(0),
      workerSync(0),
#else
      workSerial(0),
      workStepsTodo(0),
#endif
      shouldStop(false),
      workerInitPending(false),
      f(makeFunctor(w)) {
    initThreads();
  }
  ThreadRunner();
  ~ThreadRunner();

  template <class WORKER> void start(WORKER&& w) {
    if (f) delete f;
    f = makeFunctor(w);
    start();
  }
  void start();
  void join();

private:
  void initThreads();
  void workFunction(int id);

#if USE_QTHREADS
  std::vector<QThread*> threads;
#else
  std::vector<boost::thread*> threads;
#endif

#if USE_SEMAPHORE_SYNC
  QSemaphore workerPermission;
  QSemaphore workerSync;
#else
  boost::mutex waitMutex;
  boost::condition_variable condition;
  int workSerial;
  int workStepsTodo;
#endif

  bool shouldStop;
  int workerInitPending;

  class BaseThreadFunctor {
  public:
    virtual void run(int threadId)=0;
    //virtual void stop()=0;
    virtual void init(int numberOfThreads)=0;
    virtual void done(int numberOfThreads)=0;
  };

  template <typename WORKER> class ThreadFunctor: public BaseThreadFunctor {
  public:

    ThreadFunctor(WORKER&& w): worker(static_cast<WORKER&&>(w)) {}


    template <typename T> struct make { static T&& f(); };


    // ##################### Call Worker ###########################################
    template <class T> void callWorker(int threadId, decltype(make<T>::f()(0))* = nullptr) {
      worker(threadId);
    }
    template <class T> void callWorker(int , decltype(make<T>::f()())* = nullptr) {
      worker();
    }
    virtual void run(int threadNumber) {
      callWorker<WORKER>(threadNumber);
    }

    // #################### Call Stop ################################################
    /*template <class T> void callStop(int, decltype(make<T>::f().stop())* = nullptr) {
      worker.stop();
    }
    template <class T> void callStop(...) {}
    virtual void stop() {
      callStop<WORKER>(0);
    }*/


    // #################### Call Init  ################################################
    template <class T> void callInit(int numberOfThreads, decltype(make<T>::f().init(0))* = nullptr) {
      worker.init(numberOfThreads);
    }
    template <class T> void callInit(int, decltype(make<T>::f().init())* = nullptr) {
      worker.init();
    }
    template <class T> void callInit(...) {}
    virtual void init(int numberOfThreads) {
      callInit<WORKER>(numberOfThreads);
    }

    // #################### Call Done ################################################
    template <class T> void callDone(int numberOfThreads, decltype(make<T>::f().done(0))* = nullptr) {
      worker.done(numberOfThreads);
    }
    template <class T> void callDone(int, decltype(make<T>::f().done())* = nullptr) {
      worker.done();
    }
    template <class T> void callDone(...) { }
    virtual void done(int numberOfThreads) {
      callDone<WORKER>(numberOfThreads);
    }

  private:
    WORKER worker;
  };

#if USE_QTHREADS
  class WorkerThread: public QThread {
  public:
    WorkerThread(ThreadRunner* _tr, int _id): tr(_tr), id(_id) {}
    void run() { tr->workFunction(id); }
    ThreadRunner* tr;
    int id;
  };
#endif

  template <typename WORKER> static ThreadRunner::BaseThreadFunctor* makeFunctor(WORKER&& w) {
    return new ThreadRunner::ThreadFunctor< WORKER >(w);
  }

  static ThreadRunner::BaseThreadFunctor* makeFunctor(void (*f)());

  BaseThreadFunctor* f;
};


#endif // THREADRUNNER_H
