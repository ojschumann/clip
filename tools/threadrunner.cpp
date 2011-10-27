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


#include "threadrunner.h"

#include <iostream>

ThreadRunner::ThreadRunner():
#if USE_SEMAPHORE_SYNC
    workerPermission(0),
    workerSync(0),
#else
    workSerial(0),
    workStepsTodo(0),
#endif
    shouldStop(false),
    workerInitPending(false),
    f(nullptr) {
  initThreads();
}


void ThreadRunner::initThreads() {
#if USE_QTHREADS
  int N = std::max(QThread::idealThreadCount(), 1);
#else
  int N = std::max(boost::thread::hardware_concurrency(), 1u);
#endif
  for (int n=0; n<N; n++) {
#if USE_QTHREADS
    QThread* t = new WorkerThread(this, n);
    t->start();
    threads.push_back(t);
#else
    threads.push_back(new boost::thread(&ThreadRunner::workFunction, this, n));
#endif
  }
#if !USE_SEMAPHORE_SYNC
  join();
#endif
}


ThreadRunner::~ThreadRunner() {
#if USE_SEMAPHORE_SYNC
  shouldStop = true;
  workerPermission.release(threads.size());
#else
  {
    boost::unique_lock<boost::mutex> lock(waitMutex);
    shouldStop = true;
  }
  condition.notify_all();
#endif

  for (unsigned int n=0; n<threads.size(); n++) {
#if USE_QTHREADS
    threads[n]->wait();
#else
    threads[n]->join();
#endif
    delete threads[n];
  }

  delete f;
}

ThreadRunner::BaseThreadFunctor* ThreadRunner::makeFunctor(void(*f)()) {
  return new ThreadFunctor<void(*)()>(static_cast<void(*&&)()>(f));
}


void ThreadRunner::workFunction(int id) {

  while (1) {
#if USE_SEMAPHORE_SYNC
    workerPermission.acquire();
    if (shouldStop) {
      workerSync.release(1);
      return;
    }
#else
    {
      boost::unique_lock<boost::mutex> lock(waitMutex);
      --workStepsTodo;

      condition.notify_all();

      int lastWorkSerial = workSerial;
      while (lastWorkSerial == workSerial) {
        condition.wait(lock);
        if (shouldStop)
          return;
      }

      --workStepsTodo;
    }
#endif

    // Call Worker here
    if (f) f->run(id);

#if USE_SEMAPHORE_SYNC
    workerSync.release();
#endif
  }
}

void ThreadRunner::start() {
#if !USE_SEMAPHORE_SYNC
  boost::unique_lock<boost::mutex> lock(waitMutex);
  ++workSerial;
  workStepsTodo = 2*threads.size();
#endif

  if (f) {
    f->init(threads.size());
    workerInitPending = true;
  }
#if USE_SEMAPHORE_SYNC
  workerPermission.release(threads.size());
#else
  condition.notify_all();
#endif
}

void ThreadRunner::join() {
  //if (f) f->stop();

#if USE_SEMAPHORE_SYNC
  workerSync.acquire(threads.size());
#else
  boost::unique_lock<boost::mutex> lock(waitMutex);
  while (workStepsTodo!=0) {
    condition.wait(lock);
  }
#endif

  if (f && workerInitPending) {
    f->done(threads.size());
    workerInitPending = false;
  }
}






