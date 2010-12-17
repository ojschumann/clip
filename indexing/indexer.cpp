#include "indexer.h"

#include <QThreadPool>
#include <QMutexLocker>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <QtAlgorithms>

#include "tools/vec3D.h"
#include "tools/mat3D.h"
#include "tools/optimalrotation.h"


using namespace std;



Indexer::Indexer(QList<Vec3D> _spotMarkerNormals, QList<Vec3D> _zoneMarkerNormals, const Mat3D& _MReal, const Mat3D& _MReziprocal, double maxAngularDeviation, double _maxHKLDeviation, int _maxHKL, QList< TMat3D<int> > _lauegroup):
    QObject(),
    QRunnable(),
    candidates(_MReal, _MReziprocal),
    spotMarkerNormals(_spotMarkerNormals),
    zoneMarkerNormals(_zoneMarkerNormals),
    MReal(_MReal),
    MReziprocal(_MReziprocal),
    maxHKLDeviation(_maxHKLDeviation),
    maxHKL(_maxHKL)
{

  MRealInv = MReziprocal.transposed();
  MReziprocalInv = MReal.transposed();

  foreach (TMat3D<int> R, _lauegroup) {
    lauegroup << MReal * R.toType<double>() * MRealInv;
  }

  shouldStop=false;

  for (int i=0; i<spotMarkerNormals.size(); i++) {
    for (int j=0; j<i; j++) {
      spotSpotAngles.append(AngleInfo(spotMarkerNormals.at(i), Spot, i, spotMarkerNormals.at(j), Spot, j, maxAngularDeviation));
    }
  }
  qSort(spotSpotAngles);
  for (int i=0; i<zoneMarkerNormals.size(); i++) {
    for (int j=0; j<i; j++) {
      zoneZoneAngles.append(AngleInfo(zoneMarkerNormals.at(i), Zone, i, zoneMarkerNormals.at(j), Zone, j, maxAngularDeviation));
    }
  }
  qSort(zoneZoneAngles);
  for (int i=0; i<spotMarkerNormals.size(); i++) {
    for (int j=0; j<zoneMarkerNormals.size(); j++) {
      spotZoneAngles.append(AngleInfo(spotMarkerNormals.at(i), Spot, i, zoneMarkerNormals.at(j), Zone, j, maxAngularDeviation));
    }
  }
  qSort(spotZoneAngles);

  connect(&candidates, SIGNAL(nextMajorIndex(int)), this, SIGNAL(nextMajorIndex(int)));
  connect(&candidates, SIGNAL(progessInfo(int)), this, SIGNAL(progressInfo(int)));
}


void Indexer::run() {
  QThreadPool::globalInstance()->tryStart(this);

  forever {
    int i = candidatePos.fetchAndAddOrdered(1);
    CandidateGenerator::Candidate c1 = candidates.getCandidate(i);
    for (int j=0; j<i; j++) {
      if (shouldStop) return;
      CandidateGenerator::Candidate c2 = candidates.getCandidate(j);

      checkPossibleAngles(c1, c2, spotSpotAngles);
      checkPossibleAngles(c1, c2, zoneZoneAngles);
      checkPossibleAngles(c1, c2, spotZoneAngles);
    }
  }
}

void Indexer::checkPossibleAngles(const CandidateGenerator::Candidate& c1, const CandidateGenerator::Candidate& c2, QList<AngleInfo> angles) {
  if (angles.empty()) return;

  Vec3D v1 = (angles.at(0).type1==Spot)?c1.reziprocalNormal:c1.realNormal;
  Vec3D v2 = (angles.at(0).type2==Spot)?c2.reziprocalNormal:c2.realNormal;

  double cosAng = v1*v2;

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
    checkGuess(c1, c2, angles.at(n));
    checkGuess(c2, c1, angles.at(n));
  }
}


void Indexer::checkGuess(const CandidateGenerator::Candidate& c1, const CandidateGenerator::Candidate& c2, const AngleInfo &a) {
  // Prepare Best Rotation Matrix from c1,c2 -> a(1) a(2)

  Vec3D v1 = (a.type1==Spot)?c1.reziprocalNormal:c1.realNormal;
  Vec3D v2 = (a.type2==Spot)?c2.reziprocalNormal:c2.realNormal;
  Mat3D R(VectorPairRotation(a.v1, a.v2, v1, v2));

  Solution solution;
  solution.indexingRotation = R;
  solution.maxHKL=maxHKL;
  solution.maxHklDeviation = maxHKLDeviation;
  solution.MReal = MReal;
  solution.MRealInv = MRealInv;
  solution.MReziprocal = MReziprocal;
  solution.MReziprocalInv = MReziprocalInv;

  if (!solution.addMarkers(spotMarkerNormals, Spot, a, c1.index, c2.index)) return;
  if (!solution.addMarkers(zoneMarkerNormals, Zone, a, c1.index, c2.index)) return;

  solution.calcBestRotation();

  Mat3D bestinv(solution.bestRotation.transposed());
  int n=0;
  uniqLock.lockForRead();
  bool duplicate = symmetryEquivalentSolutionPresent(bestinv, n);
  uniqLock.unlock();
  if (duplicate) return;

  uniqLock.lockForWrite();
  duplicate = symmetryEquivalentSolutionPresent(bestinv, n);
  if (!duplicate) {
    SolutionInfo info;
    info.R=solution.bestRotation;
    info.score = solution.hklDeviationSum();
    uniqSolutions << info;
    emit publishSolution(solution);
  }
  uniqLock.unlock();
}

bool Indexer::symmetryEquivalentSolutionPresent(const Mat3D &R, int &n) {
  for (; n<uniqSolutions.size(); n++) {
    Mat3D T(R*uniqSolutions.at(n).R);
    foreach (Mat3D R, lauegroup)  {
      if ((R-T).sqSum()<1e-6) {
        return true;
      }
    }
  }
  return false;
}




Indexer::AngleInfo::AngleInfo(const Vec3D &_v1, Indexer::MarkerType t1, int i1, const Vec3D &_v2, Indexer::MarkerType t2, int i2, double maxDeviation):
    v1(_v1),
    type1(t1),
    index1(i1),
    v2(_v2),
    type2(t2),
    index2(i2)
{
  cosAng=v1*v2;
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
  //QMutexLocker lock(&indexMutex);
  shouldStop=true;
}
