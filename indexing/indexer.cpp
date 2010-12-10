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



Indexer::Indexer(QList<Vec3D> _spotMarkerNormals, QList<Vec3D> _zoneMarkerNormals, const Mat3D& _MReal, const Mat3D& _MReziprocal):
    QObject(),
    QRunnable(),
    candidates(_MReal, _MReziprocal),
    spotMarkerNormals(_spotMarkerNormals),
    zoneMarkerNormals(_zoneMarkerNormals),
    MReal(_MReal),
    MReziprocal(_MReziprocal)
{
  nextProgressSignal=0;

  shouldStop=false;


  for (int i=0; i<spotMarkerNormals.size(); i++) {
    for (int j=0; j<i; j++) {
      spotSpotAngles.append(AngleInfo(spotMarkerNormals.at(i), spotMarkerNormals.at(j), 0.017));
    }
  }
  qSort(spotSpotAngles);
  for (int i=0; i<zoneMarkerNormals.size(); i++) {
    for (int j=0; j<i; j++) {
      zoneZoneAngles.append(AngleInfo(zoneMarkerNormals.at(i), zoneMarkerNormals.at(j), 0.017));
    }
  }
  qSort(zoneZoneAngles);
  for (int i=0; i<spotMarkerNormals.size(); i++) {
    for (int j=0; j<zoneMarkerNormals.size(); j++) {
      spotZoneAngles.append(AngleInfo(spotMarkerNormals.at(i), zoneMarkerNormals.at(j), 0.017));
    }
  }
  qSort(spotZoneAngles);
}


void Indexer::run() {
  //QThreadPool::globalInstance()->tryStart(this);

  forever {
    int i = candidatePos.fetchAndAddOrdered(1);
    for (int j=0; j<i; j++) {
      if (shouldStop) return;
      CandidateGenerator::Candidate c1 = candidates.getCandidate(i);
      CandidateGenerator::Candidate c2 = candidates.getCandidate(j);

      checkPossibleAngles(c1.reziprocalNormal, c2.reziprocalNormal, spotSpotAngles, c1, c2);
      checkPossibleAngles(c1.realNormal, c2.realNormal, zoneZoneAngles, c1, c2);
      checkPossibleAngles(c1.reziprocalNormal, c2.realNormal, spotZoneAngles, c1, c2);
    }
    if (i>300) return;
  }
}

void Indexer::checkPossibleAngles(const Vec3D& v1, const Vec3D& v2, QList<AngleInfo> angles, const CandidateGenerator::Candidate& c1, const CandidateGenerator::Candidate& c2) {
  if (angles.empty()) return;

  double cosAng = v1*v2;

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

  for ( ; minIdx<angles.size() && cosAng>=angles.at(minIdx).lowerBound; minIdx++) {
    checkGuess(v1, v2, angles.at(minIdx));
    checkGuess(v2, v1, angles.at(minIdx));
  }
}


void Indexer::checkGuess(const Vec3D &v1, const Vec3D &v2, const AngleInfo &a) {

  // Prepare Best Rotation Matrix from c1,c2 -> a(1) a(2)

  Mat3D R(VectorPairRotation(a.v1, a.v2, v1, v2));

  // Try Indexation of missing reflexions

  foreach (Vec3D n, spotMarkerNormals) {
    Vec3D v = MReal*R*n;
  }

  foreach (Vec3D n, zoneMarkerNormals) {
    Vec3D v = MReziprocal*R*n;
  }




  /*


  Solution s;
  s.indexingRotation=R;

  for (unsigned int n=0; n<p.markerNormals.size(); n++) {
    SolutionItem si;
    si.initialIndexed=true;
#ifdef __DEBUG__
    si.rotatedMarker=R*p.markerNormals.at(n);
#endif
    bool ok=true;
    if (n==a.index1) {
      si.h=c1.h;
      si.k=c1.k;
      si.l=c1.l;
    } else if (n==a.index2) {
      si.h=c2.h;
      si.k=c2.k;
      si.l=c2.l;
    } else {
      Vec3D hklVect(OMatInv*R*p.markerNormals.at(n));
      double max=MAX(MAX(fabs(hklVect(0)),fabs(hklVect(1))),fabs(hklVect(2)));
      hklVect*=1.0/max;
      si.initialIndexed=false;

      for (unsigned int order=1; order<=p.maxOrder; order++) {
        Vec3D t(hklVect*order);
#ifdef __DEBUG__
        si.rationalHkl=t;
#endif
        ok=true;
        for (unsigned int i=3; i--; ) {
          if (fabs(fabs(t(i))-round(fabs(t(i))))>p.maxIntegerDeviation) {
            ok=false;
            break;
          }
        }
        if (ok) {
          si.h=(int)round(t(0));
          si.k=(int)round(t(1));
          si.l=(int)round(t(2));
          break;
        }
      }
    }
    if (ok) {
      s.items.append(si);
    } else {
      break;
    }
  }

  if (p.markerNormals.size()==s.items.size()) {
    // yes, we have a solution!!!
    O.reset();
    for (unsigned int n=s.items.size(); n--; ) {
      SolutionItem& si=s.items[n];
      Vec3D v(si.h, si.k, si.l);
      v=p.orientationMatrix*v;
      v.normalize();
      si.latticeVector=v;
      O.addVectorPair(v,p.markerNormals[n]);
    }

    s.bestRotation=O.getOptimalRotation();

    if (newSolution(s.bestRotation)) {
      for (unsigned int n=s.items.size(); n--; ) {
        SolutionItem& si=s.items[n];
        si.rotatedMarker=s.bestRotation*p.markerNormals.at(n);
        otimizeScale(si);
      }
      emit publishSolution(s);
    }
  }*/

}

void Indexer::otimizeScale(SolutionItem& si) {
  /*Vec3D hkl(si.h,si.k,si.l);
  si.rationalHkl=OMatInv*si.rotatedMarker;
  si.rationalHkl*=hkl*si.rationalHkl/si.rationalHkl.norm_sq();
  */
  //sum (hkl-scale*rhkl)^2 = min
  // dsum/scale = 2sum (hkl_i-s*rhkl_i)*rhkl_i == 0!
  // => s* sum( rhkl_i^2 ) = sum ( rhkl_i * hkl_i )
}

bool Indexer::newSolution(const Mat3D& M) {
  //TODO: This is possibly a performance lock. The threads might serialize here
  /*QMutexLocker lock(&solRotLock);
  Mat3D T1(M.transposed());
  for (unsigned int n=solutionRotations.size(); n--; ) {
    Mat3D T2(solutionRotations.at(n)*T1);
    for (unsigned int m=p.pointGroup.size(); m--; ) {
      Mat3D T3(T2-p.pointGroup.at(m));
      if (T3.sqSum()<1e-10) {
        return false;
      }
    }
  }
  solutionRotations.append(M);*/
  return true;
}



Indexer::AngleInfo::AngleInfo(const Vec3D &_v1, const Vec3D &_v2, double maxDeviation):
    v1(_v1),
    v2(_v2)
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

bool Indexer::AngleInfo::cmpAngleInfoLowerBound(const AngleInfo &a1, const AngleInfo &a2) {
  return a1.lowerBound<a2.lowerBound;
}
bool Indexer::AngleInfo::cmpAngleInfoUpperBound(const AngleInfo &a1, const AngleInfo &a2) {
  return a1.upperBound<a2.upperBound;
}

void Indexer::stop() {
  //QMutexLocker lock(&indexMutex);
  shouldStop=true;
}
