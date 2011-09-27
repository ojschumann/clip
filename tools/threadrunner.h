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

#ifndef THREADRUNNER_H
#define THREADRUNNER_H

#include <boost/thread.hpp>
#include <vector>
#include <thread>



class ThreadRunner {
public:
  template <class WORKER> ThreadRunner(WORKER&& w):
      shouldStop(false),
      workSerial(0),
      workerInitPending(false),
      workStepsTodo(0),
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

  std::vector<boost::thread*> threads;
  boost::mutex waitMutex;
  boost::condition_variable condition;

  bool shouldStop;
  int workSerial;
  int workerInitPending;
  int workStepsTodo;

  class BaseThreadFunctor {
  public:
    virtual void run()=0;
    virtual void init()=0;
    virtual void done()=0;
  };

  template <typename WORKER> class ThreadFunctor: public BaseThreadFunctor {
  public:
    ThreadFunctor(WORKER&& w): worker(static_cast<WORKER&&>(w)) {}
    virtual void run() { worker(); }
    template <typename T> void callInit(decltype(&boost::remove_reference<T>::type::init)) { worker.init(); }
    template <typename T> void callInit(...) {  }
    virtual void init() { callInit<WORKER>(0); }

    template <typename T> void callDone(decltype(&boost::remove_reference<T>::type::init)) { worker.done(); }
    template <typename T> void callDone(...) {  }
    virtual void done() { callDone<WORKER>(0); }

  private:
    WORKER worker;
  };
  template <typename WORKER> ThreadRunner::BaseThreadFunctor* makeFunctor(WORKER&& w) {
    return new ThreadRunner::ThreadFunctor< WORKER >(w);
  }

  ThreadRunner::BaseThreadFunctor* makeFunctor(void (*f)());

  BaseThreadFunctor* f;
};


#endif // THREADRUNNER_H
