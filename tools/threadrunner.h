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

#include <boost/thread.hpp>
#include <vector>
#include <thread>

#include "config.h"

#define USE_QTHREADS 0
#define USE_SEMAPHORE_SYNC 1

#if USE_QTHREADS
#include <QThread>
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
    virtual void init(int numberOfThreads)=0;
    virtual void done(int numberOfThreads)=0;
  };

  template <typename WORKER> class ThreadFunctor: public BaseThreadFunctor {
  public:
    typedef typename boost::remove_cv<typename boost::remove_reference<WORKER>::type>::type PURE_WORKER;

    ThreadFunctor(WORKER&& w): worker(static_cast<WORKER&&>(w)) {}

    template <typename T> struct Helper {};

    template <class T> void callWorker(int threadId, Helper<decltype((*((T*)nullptr))(0))>*) {
      worker(threadId);
    }
    template <typename T> void callWorker(int, Helper<decltype((*((T*)nullptr))())>*) {
      worker();
    }
    virtual void run(int threadNumber) {
      //worker();
      callWorker<typename boost::remove_cv<typename boost::remove_reference<WORKER>::type>::type>(threadNumber, nullptr);
    }

    template <class T> void callInit(int numberOfThreads, decltype(((T*)nullptr)->init(0)) (T::*)(int)) {
      //std::cout << "init auto (T::*)(int)" << std::endl;
      worker.init(numberOfThreads);
    };
    template <class T> void callInit(int numberOfThreads, decltype(((T*)nullptr)->init()) (T::*)()) {
      //std::cout << "init auto (T::*)() sig" << std::endl;
      worker.init();
    };
    template <class T> void callInit(...) {
      //std::cout << "init catch" << std::endl;
    };
    virtual void init(int numberOfThreads) {
      callInit<typename boost::remove_cv<typename boost::remove_reference<WORKER>::type>::type>(numberOfThreads, 0);
    }

    template <class T> void callDone(int numberOfThreads, decltype(((T*)nullptr)->done(0)) (T::*)(int)) {
      //std::cout << "done auto (T::*)(int)" << std::endl;
      worker.done(numberOfThreads);
    };
    template <class T> void callDone(int numberOfThreads, decltype(((T*)nullptr)->done()) (T::*)()) {
      //std::cout << "done auto (T::*)()" << std::endl;
      worker.done();
    };
    template <class T> void callDone(...) {
      //std::cout << "done catch" << std::endl;
    };
    virtual void done(int numberOfThreads) {
      callDone<typename boost::remove_cv<typename boost::remove_reference<WORKER>::type>::type>(numberOfThreads, 0);
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
