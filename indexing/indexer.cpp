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
#include "indexing/marker.h"


using namespace std;



Indexer::Indexer(QList<AbstractMarkerItem*> crystalMarkers, const Mat3D& _MReal, const Mat3D& _MReziprocal, double maxAngularDeviation, double _maxHKLDeviation, int _maxHKL, QList< TMat3D<int> > _lauegroup):
    QObject(),
    QRunnable(),
    candidatePos(0),
    loopCounter(0),
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
  loopTimer.start();
  runTimer.start();
}


void Indexer::run() {
  if (multithreaded)
    QThreadPool::globalInstance()->tryStart(this);

  ThreadLocalData localData;
  localData.markers = globalMarkers;
  for (int i=0; i<localData.markers.size(); i++)
    localData.markers[i].setMatrices(localData.spotNormalToIndex, localData.zoneNormalToIndex, MReziprocal, MReal);

  int loop=0;
  forever {
    int i = candidatePos.fetchAndAddOrdered(1);
    CandidateGenerator::Candidate c1 = candidates.getCandidate(i);
    QList<CandidateGenerator::Candidate> cList = candidates.getCandidateList(i);
    for (int j=0; j<cList.size(); j++) {
      if (shouldStop) return;
      if (uniqSolutions.size()>200) return;
      // Be nice to the GUI Thread
      loop++;
      if ((loop%10000)==0)
        QThread::yieldCurrentThread();

      int testNr = loopCounter.fetchAndAddRelaxed(1);
      if (testNr%10000==0) {
        int dt = loopTimer.restart();
        cout << testNr << " " << loop << " " << dt << " " << 10000000.0/dt << " " << 1000.0*testNr/runTimer.elapsed() << endl;
      }


      CandidateGenerator::Candidate c2 = cList.at(j);

      checkPossibleAngles(c1.spot(), c2.spot(), spotSpotAngles, localData);
      checkPossibleAngles(c1.zone(), c2.zone(), zoneZoneAngles, localData);
      checkPossibleAngles(c1.spot(), c2.zone(), spotZoneAngles, localData);
    }
  }
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
    Mat3D T = optRot.getOptimalRotation();
    if (optRot.getOptimalRotation()==R) break;
    R = optRot.getOptimalRotation();
  }

  if (loops>5)
    cout << "Loops " << loops << endl;

  Solution solution;
  solution.bestRotation = R;
  solution.hklDeviation = 0.0;
  foreach (Marker m, localData.markers) {
    solution.markerIdx << m.getIntegerIndex();
    solution.hklDeviation += m.getIndexDeviationScore();
  }

  Mat3D bestinv(solution.bestRotation.transposed());
  int n=0;
  uniqLock.lockForRead();
  bool duplicate = symmetryEquivalentSolutionPresent(bestinv, solution.hklDeviation, n);
  uniqLock.unlock();
  if (duplicate) return;

  uniqLock.lockForWrite();
  duplicate = symmetryEquivalentSolutionPresent(bestinv, solution.hklDeviation, n);
  if (!duplicate) {
    uniqSolutions << solution;
    //cout << c1.index(0) << "," << c1.index(1) << "," << c1.index(2) << " + " << c2.index(0) << "," << c2.index(1) << "," << c2.index(2) << endl;
    emit publishSolution(solution);
  }
  uniqLock.unlock();
}

bool Indexer::symmetryEquivalentSolutionPresent(const Mat3D &R, double hklDeviation, int &n) {
  for (; n<uniqSolutions.size(); n++) {
    if ((fabs(uniqSolutions.at(n).hklDeviation - hklDeviation) / hklDeviation)<1e-4) {;
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
  //QMutexLocker lock(&indexMutex);
  shouldStop=true;
}
