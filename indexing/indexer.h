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

#ifndef INDEXER_H
#define INDEXER_H

#include <QRunnable>
#include <QMutex>
#include <QAtomicInt>
#include <QReadWriteLock>
#include <QTime>

#include "tools/vec3D.h"
#include "indexing/candidategenerator.h"
#include "indexing/marker.h"

class Solution;

class Indexer: public QObject {
  Q_OBJECT
public:
  class AngleInfo {
  public:
    AngleInfo(int i1, int i2, const QList<Marker>& markers, double maxDeviation);
    bool operator<(const AngleInfo &o) const;

    double lowerBound;
    double cosAng;
    double upperBound;

    int index1;
    int index2;
  };

  Indexer(QList<AbstractMarkerItem*> crystalMarkers, const Mat3D& MReal, const Mat3D& MReziprocal, double _maxAngularDeviation, double _maxHKLDeviation, int _maxHKL, QList< TMat3D<int> > _lauegroup, QObject* parent);
  virtual ~Indexer();

  void run();
  void operator()() { run(); }

public slots:
  void stop();

signals:
  void publishSolution(Solution s);
  void publishMultiSolutions(QList<Solution> s);
  void progressInfo(int);
  void nextMajorIndex(int);

protected:
  struct ThreadLocalData {
    QList<Marker> markers;
    Mat3D spotNormalToIndex;
    Mat3D zoneNormalToIndex;
    int solutionsPublishedInRateCycle;
    bool publishSingleSolution;
    QList<Solution> unpublishedSolutions;
  };

  void checkGuess(const CandidateGenerator::Candidate&, const CandidateGenerator::Candidate&, const AngleInfo &, ThreadLocalData&);
  void checkPossibleAngles(const CandidateGenerator::Candidate&, const CandidateGenerator::Candidate&, QList<AngleInfo>, ThreadLocalData&);

  QAtomicInt candidatePos;
  CandidateGenerator candidates;

  bool shouldStop;
  QAtomicInt runningThreads;

  QList<AngleInfo> spotSpotAngles;
  QList<AngleInfo> spotZoneAngles;
  QList<AngleInfo> zoneZoneAngles;

  QList<Marker> globalMarkers;

  Mat3D MReal;
  Mat3D MRealInv;
  Mat3D MReziprocal;
  Mat3D MReziprocalInv;

  double maxHKLDeviation;

  QList<Solution> uniqSolutions;
  QList<Mat3D> lauegroup;
  QReadWriteLock uniqLock;

  bool symmetryEquivalentSolutionPresent(const Mat3D& R, double hklDeviation, int& checkFrom);
};

#include "indexing/solution.h"

#endif
