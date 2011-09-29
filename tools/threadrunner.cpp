/**************************************************************************
  Copyright (C) 2011 schumann

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
**************************************************************************/


#include "threadrunner.h"

#include <iostream>

ThreadRunner::ThreadRunner():
    shouldStop(false),
    workSerial(0),
    workerInitPending(false),
    workStepsTodo(0),
    f(nullptr) {
  initThreads();
}


void ThreadRunner::initThreads() {
  int N = std::max(boost::thread::hardware_concurrency(), 1u);
  workStepsTodo = N;
  for (int n=0; n<N; n++) {
    threads.push_back(new boost::thread(&ThreadRunner::workFunction, this, n));
  }
  join();
}


ThreadRunner::~ThreadRunner() {
  {
    boost::unique_lock<boost::mutex> lock(waitMutex);
    shouldStop = true;
  }
  condition.notify_all();

  for (unsigned int n=0; n<threads.size(); n++) {
    threads[n]->join();
    delete threads[n];
  }

  delete f;
}

ThreadRunner::BaseThreadFunctor* ThreadRunner::makeFunctor(void(*f)()) {
  return new ThreadFunctor<void(*)()>(static_cast<void(*&&)()>(f));
}


void ThreadRunner::workFunction(int id) {

  while (1) {
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
      //std::cout << "Hallo Welt " << id << " " << boost::this_thread::get_id() << " " << this << " " << f << std::endl;
    }

    // Call Worker here
    if (f) f->run(id);

  }
}

void ThreadRunner::start() {
  boost::unique_lock<boost::mutex> lock(waitMutex);
  ++workSerial;
  workStepsTodo = 2*threads.size();

  if (f) {
    f->init(threads.size());
    workerInitPending = true;
  }

  condition.notify_all();
}

void ThreadRunner::join() {
  boost::unique_lock<boost::mutex> lock(waitMutex);
  while (workStepsTodo!=0) {
    condition.wait(lock);
  }

  if (f) {
    if (workerInitPending) {
      f->done(threads.size());
      workerInitPending = false;
    }
  }
}






