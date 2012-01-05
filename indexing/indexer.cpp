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

#include "indexer.h"

#include <QMutexLocker>
 

#include <cmath>
#include <QtAlgorithms>
#include <QDebug>
#include <QThread>

#include "tools/vec3D.h"
#include "tools/mat3D.h"
#include "tools/optimalrotation.h"
#include "indexing/marker.h"


using namespace std;



Indexer::Indexer(QList<AbstractMarkerItem*> crystalMarkers, const Mat3D& _MReal, const Mat3D& _MReziprocal, double maxAngularDeviation, double _maxHKLDeviation, int _maxHKL, QList< TMat3D<int> > _lauegroup, QObject* parent):
    QObject(parent),
    candidatePos(0),
    candidates(_MReal, _MReziprocal),
    MReal(_MReal),
    MReziprocal(_MReziprocal),
    maxHKLDeviation(_maxHKLDeviation)
{

  MRealInv = MReziprocal.transposed();
  MReziprocalInv = MReal.transposed();

  foreach (TMat3D<int> R, _lauegroup) {
    lauegroup << MReal * R.toType<double>() * MRealInv;
  }

  shouldStop=false;
  runningThreads = 0;

  foreach (AbstractMarkerItem* m, crystalMarkers)
    globalMarkers << Marker(m->getMarkerNormal(), m->getType(), _maxHKL);

  for (int i=0; i<globalMarkers.size(); i++) {
    for (int j=0; j<i; j++) {
      if (globalMarkers.at(i).getType()==Marker::SpotMarker && globalMarkers.at(j).getType()==Marker::SpotMarker) {
        spotSpotAngles.append(AngleInfo(i, j, globalMarkers, maxAngularDeviation));
      } else if (globalMarkers.at(i).getType()==Marker::ZoneMarker && globalMarkers.at(j).getType()==Marker::ZoneMarker) {
        zoneZoneAngles.append(AngleInfo(i, j, globalMarkers, maxAngularDeviation));
      } else {
        spotZoneAngles.append(AngleInfo(i, j, globalMarkers, maxAngularDeviation));
      }
    }
  }
  qSort(spotSpotAngles);
  qSort(zoneZoneAngles);
  qSort(spotZoneAngles);

  connect(&candidates, SIGNAL(nextMajorIndex(int)), this, SIGNAL(nextMajorIndex(int)));
  connect(&candidates, SIGNAL(progessInfo(int)), this, SIGNAL(progressInfo(int)));
}

Indexer::~Indexer() {
}

void Indexer::run() {
  runningThreads.ref();

  ThreadLocalData localData;
  localData.solutionsPublishedInRateCycle = 0;
  localData.publishSingleSolution = true;
  localData.markers = globalMarkers;
  for (int i=0; i<localData.markers.size(); i++)
    localData.markers[i].setMatrices(localData.spotNormalToIndex, localData.zoneNormalToIndex, MReziprocal, MReal);

  QTime nice;
  nice.start();
  forever {
    int i = candidatePos.fetchAndAddOrdered(1);
    QList<CandidateGenerator::Candidate> cList = candidates.getCandidateList(i+1);
    CandidateGenerator::Candidate c1 = cList.takeLast();
    for (int j=0; j<cList.size(); j++) {
      if (shouldStop) {
        runningThreads.deref();
        return;
      }
      if (nice.elapsed()>100) {
        // Be nice to the GUI Thread
        QThread::yieldCurrentThread();
        nice.restart();
        localData.publishSingleSolution = ((localData.solutionsPublishedInRateCycle + localData.unpublishedSolutions.size()) <= 10);
        if (localData.unpublishedSolutions.size()>0) {
          emit publishMultiSolutions(localData.unpublishedSolutions);
          localData.unpublishedSolutions.clear();
        }
        localData.solutionsPublishedInRateCycle = 0;
      }

      CandidateGenerator::Candidate c2 = cList.at(j);

      checkPossibleAngles(c1.spot(), c2.spot(), spotSpotAngles, localData);
      checkPossibleAngles(c1.zone(), c2.zone(), zoneZoneAngles, localData);
      checkPossibleAngles(c1.spot(), c2.zone(), spotZoneAngles, localData);
    }
  }
  runningThreads.deref();
}

void Indexer::checkPossibleAngles(const CandidateGenerator::Candidate& c1, const CandidateGenerator::Candidate& c2, QList<AngleInfo> angles, ThreadLocalData& localData) {
  if (angles.empty()) return;

  double cosAng = c1.normal*c2.normal;

  // Binary search for first AngleInfo that has upperBound>cosAng
  int minIdx = 0;
  int maxIdx = angles.size();
  while (maxIdx!=minIdx) {
    int chkIdx = (minIdx+maxIdx)/2;
    if (cosAng>angles.at(chkIdx).upperBound) {
      minIdx = chkIdx+1;
    } else {
      maxIdx = chkIdx;
    }
  }

  for (int n=minIdx; n<angles.size() && cosAng>=angles.at(n).lowerBound; n++) {
    checkGuess(c1, c2, angles.at(n), localData);
    checkGuess(c2, c1, angles.at(n), localData);
  }
}


void Indexer::checkGuess(const CandidateGenerator::Candidate& c1, const CandidateGenerator::Candidate& c2, const AngleInfo &a, ThreadLocalData& localData) {
  // Prepare Best Rotation Matrix from c1,c2 -> a(1) a(2)

  Mat3D R = VectorPairRotation(c1.normal,
                               c2.normal,
                               localData.markers.at(a.index1).getMarkerNormal(),
                               localData.markers.at(a.index2).getMarkerNormal());

  int loops = 0;
  forever {
    loops++;
    localData.spotNormalToIndex = MReziprocalInv * R.transposed();
    localData.zoneNormalToIndex = MRealInv * R.transposed();

    OptimalRotation optRot;
    for (int i=0; i<localData.markers.size(); i++) {
      Marker& m = localData.markers[i];
      if (i==a.index1) {
        m.setIndex(c1.index);
      } else if (i==a.index2) {
        m.setIndex(c2.index);
      } else {
        m.invalidateCache();
      }

      if (m.getIndexDeviationScore()>maxHKLDeviation) return;
      optRot.addVectorPair(m.getIndexNormal(), m.getMarkerNormal());
    }
    if (optRot.getOptimalRotation()==R) break;
    if (shouldStop) return;
    if (loops>25) {
      return;
    }
    R = optRot.getOptimalRotation();
  }

  Solution solution;
  solution.bestRotation = R;
  foreach (Marker m, localData.markers) {
    solution.markerIdx << m.getIntegerIndex();
    solution.markerRationalIdx << m.getRationalIndex();
  }

  Mat3D bestinv(solution.bestRotation.transposed());
  int n=0;
  uniqLock.lockForRead();
  bool duplicate = symmetryEquivalentSolutionPresent(bestinv, solution.indexDeviation, n);
  uniqLock.unlock();
  if (duplicate) return;

  uniqLock.lockForWrite();
  duplicate = symmetryEquivalentSolutionPresent(bestinv, solution.indexDeviation, n);
  if (!duplicate) {
    uniqSolutions << solution;
    if (localData.publishSingleSolution && (localData.solutionsPublishedInRateCycle<5)) {
      localData.solutionsPublishedInRateCycle++;
      emit publishSolution(solution);
    } else {
      localData.unpublishedSolutions << solution;
    }
  }
  uniqLock.unlock();
}

bool Indexer::symmetryEquivalentSolutionPresent(const Mat3D &R, double hklDeviation, int &n) {
  for (; n<uniqSolutions.size(); n++) {
    if ((fabs(uniqSolutions.at(n).indexDeviation - hklDeviation) / hklDeviation)<1e-4) {;
      Mat3D T(R*uniqSolutions.at(n).bestRotation);
      foreach (Mat3D G, lauegroup)  {
        if ((G-T).sqSum()<1e-4) {
          return true;
        }
      }
    }
  }
  return false;
}




Indexer::AngleInfo::AngleInfo(int i1, int i2, const QList<Marker>& markers, double maxDeviation):
    index1(i1),
    index2(i2)
{
  cosAng=markers.at(index1).getMarkerNormal()*markers.at(index2).getMarkerNormal();
  double c=acos(cosAng);
  double c1=cos(c-maxDeviation);
  double c2=cos(c+maxDeviation);
  if (c1<c2) {
    lowerBound = c1;
    upperBound = c2;
  } else {
    lowerBound = c2;
    upperBound = c1;
  }
  if (fabs(c)<maxDeviation) upperBound = 1.0;
  if (fabs(c-M_PI)<maxDeviation) lowerBound = -1.0;
}


bool Indexer::AngleInfo::operator<(const AngleInfo& o) const {
  return cosAng<o.cosAng;
}

void Indexer::stop() {
  shouldStop=true;
}
