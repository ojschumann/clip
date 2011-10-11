/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

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
 **********************************************************************/

#ifndef NELDERMEAD_WORKER_H
#define NELDERMEAD_WORKER_H

#include <QObject>
#include "refinement/neldermead.h"

class NMWorker: public QObject {
  Q_OBJECT
public:
  NMWorker(Crystal* c, QObject* _parent=nullptr);
  virtual ~NMWorker();

  void doOneIteration();
  QList<double> parameterDelta();
  QList<double> parameterRelativeDelta();
  QList<double> calcDeviation();
  QString parameterName(int n);
  QList<double> bestSolution();
  double bestScore();
  double worstScore();

  void restart();
protected slots:
  void updateTransformationMatrices();
protected:
  class Vertex {
  public:
    Vertex();
    Vertex(int N);
    Vertex(const Vertex& o);
    double score;
    QVector<double> coordinates;

    bool operator<(const Vertex& o) const;
    Vertex& operator=(const Vertex& o);
    Vertex& operator+=(const Vertex& o);
    Vertex& operator-=(const Vertex& o);
    Vertex& operator*=(double scale);
    Vertex operator+(const Vertex& o);
    Vertex operator-(const Vertex& o);
    Vertex operator*(double scale) const;

    double at(int n) const;
    int size() const;
  };
  class MarkerInfo {
  public:
    MarkerInfo(AbstractMarkerItem* item);
    double score(const Mat3D& spotTransfer, const Mat3D& zoneTransfer) const;
  //protected:
    AbstractMarkerItem* marker;
    Vec3D index;
    double index_sq;
  };

  void initParameter();
  void initSimplex();

  double score();
  double score(Vertex&);
  double score_c(const Vertex&);

  // Crystal, that is used in the UI
  Crystal* liveCrystal;

  // The thing to optimize
  QList<Vertex> simplex;

  // Matrices to transfer normals to indices
  Mat3D spotTransferMatrix;
  Mat3D zoneTransferMatrix;

  // local copies of crystal and projectors
  QList<FitObject*> copiedFitObjects;
  QList<FitParameter*> parameters;
  QList<MarkerInfo> markers;

};



#endif // NELDERMEAD_WORKER_H
