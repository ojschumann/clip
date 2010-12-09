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
    spotMarkerNormals(_spotMarkerNormals),
    zoneMarkerNormals(_zoneMarkerNormals),
    MReal(_MReal),
    MReziprocal(_MReziprocal)
{
  nextProgressSignal=0;

  shouldStop=false;


  for (int i=0; i<spotMarkerNormals.size(); i++) {
    for (int j=0; j<i; j++) {
      angles.append(AngleInfo(spotMarkerNormals.at(i), spotMarkerNormals.at(j), AngleInfo::Spot, AngleInfo::Spot));
    }
  }
  for (int i=0; i<zoneMarkerNormals.size(); i++) {
    for (int j=0; j<i; j++) {
      angles.append(AngleInfo(zoneMarkerNormals.at(i), zoneMarkerNormals.at(j), AngleInfo::Zone, AngleInfo::Zone));
    }
  }
  for (int i=0; i<spotMarkerNormals.size(); i++) {
    for (int j=0; j<zoneMarkerNormals.size(); j++) {
      angles.append(AngleInfo(spotMarkerNormals.at(i), zoneMarkerNormals.at(j), AngleInfo::Spot, AngleInfo::Zone));
    }
  }
  qSort(angles);
}

void Indexer::run() {
  QThreadPool::globalInstance()->tryStart(this);

  forever {
    int i = candidatePos.fetchAndAddOrdered(1);
    for (int j=0; j<i; j++) {
      if (shouldStop) return;
      CandidateGenerator::Candidate c1 = candidates.getCandidate(i);
      CandidateGenerator::Candidate c2 = candidates.getCandidate(j);

    double cosAng=p.refs.at(i).normalLocal*p.refs.at(j).normalLocal;
    lower.upperBound=cosAng;
    QList<Indexer::AngleInfo>::iterator iter=qLowerBound(angles.begin(), angles.end(), lower, Indexer::AngleInfo::cmpAngleInfoUpperBound);
    bool ok=false;
    while (iter!=angles.end() && cosAng>=iter->lowerBound) {
      if (iter->lowerBound<=cosAng && cosAng<iter->upperBound) {
        checkGuess(p.refs.at(i), p.refs.at(j),  *iter);
        checkGuess(p.refs.at(j), p.refs.at(i),  *iter);
      }
      iter++;
    }
  }
}

#define MAX(x,y) (((x)>(y))?(x):(y))

void Indexer::checkGuess(const Reflection &c1, const Reflection &c2,  const AngleInfo &a) {
  OptimalRotation O;
  O.addVectorPair(c1.normalLocal, p.markerNormals[a.index1]);
  O.addVectorPair(c2.normalLocal, p.markerNormals[a.index2]);
  // Prepare Best Rotation Matrix from c1,c2 -> a(1) a(2)

  Mat3D R(O.getOptimalRotation());


  // Try Indexation of missing reflexions
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
  }

}

void Indexer::otimizeScale(SolutionItem& si) {
  Vec3D hkl(si.h,si.k,si.l);
  si.rationalHkl=OMatInv*si.rotatedMarker;
  si.rationalHkl*=hkl*si.rationalHkl/si.rationalHkl.norm_sq();
  //sum (hkl-scale*rhkl)^2 = min
  // dsum/scale = 2sum (hkl_i-s*rhkl_i)*rhkl_i == 0!
  // => s* sum( rhkl_i^2 ) = sum ( rhkl_i * hkl_i )
}

bool Indexer::newSolution(const Mat3D& M) {
  //TODO: This is possibly a performance lock. The threads might serialize here
  QMutexLocker lock(&solRotLock);
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
  solutionRotations.append(M);
  return true;
}


bool Indexer::nextWork(int &i, int &j) {
  QMutexLocker lock(&indexMutex);
  if (indexI>=p.refs.size() or shouldStop)
    return false;

  i=indexI;
  j=indexJ;

  if (nextProgressSignal==0) {
    int maxN=p.refs.size()*(p.refs.size()-1)/2;
    int actN=i*(i-1)/2+j;
    nextProgressSignal=maxN/1000;
    emit progressInfo(maxN, actN);
  }
  nextProgressSignal--;
  indexJ++;
  if (indexJ==indexI) {
    indexI++;
    indexJ=0;
  }
  return true;
}

Indexer::AngleInfo::AngleInfo(const Vec3D &v1, const Vec3D &v2, NormalType t1, NormalType t2) {
  cosAng=v1*v2;
  double c=acos(info.cosAng);
  double c1=cos(c+p.maxAngularDeviation);
  double c2=cos(c-p.maxAngularDeviation);
  lowerBound=(c1<c2)?c1:c2;
  upperBound=(c1>c2)?c1:c2;
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
  QMutexLocker lock(&indexMutex);
  shouldStop=true;
}
