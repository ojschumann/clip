/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

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
 **********************************************************************/

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
  virtual ~NelderMead();
  bool isRunning();
public slots:
  void start();
  void stop();
signals:
  void finished();
  void bestSolutionScore(double);
  void bestSolution(QList<double>, QList<double>);
protected slots:
  void setBestSolutionToLiveCrystal(QList<double>, QList<double>);
protected:
  class Worker;
  void run();


  // Crystal, that is used in the UI
  Crystal* liveCrystal;

  QFutureWatcher<void> threadWatcher;
  QReadWriteLock threadLock;
  bool shouldStop;
};

Q_DECLARE_METATYPE(QList<double>)
#endif // NELDERMEAD_H
