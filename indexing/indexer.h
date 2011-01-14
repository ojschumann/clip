#ifndef __INDEXER_H__
#define __INDEXER_H__

#include <QRunnable>
#include <QMutex>
#include <QAtomicInt>
#include <QReadWriteLock>
#include <QTime>
#include <QThreadStorage>

#include "tools/vec3D.h"
#include "indexing/candidategenerator.h"
#include "indexing/marker.h"

class Solution;

class Indexer: public QObject, public QRunnable {
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

  Indexer(QList<AbstractMarkerItem*> crystalMarkers, const Mat3D& MReal, const Mat3D& MReziprocal, double _maxAngularDeviation, double _maxHKLDeviation, int _maxHKL, QList< TMat3D<int> > _lauegroup);

  void run();

public slots:
  void stop();

signals:
  void publishSolution(Solution s);
  void progressInfo(int);
  void nextMajorIndex(int);

protected:
  struct ThreadLocalData {
    QList<Marker> markers;
    Mat3D spotNormalToIndex;
    Mat3D zoneNormalToIndex;
  };

  void checkGuess(const CandidateGenerator::Candidate&, const CandidateGenerator::Candidate&, const AngleInfo &, ThreadLocalData&);
  void checkPossibleAngles(const CandidateGenerator::Candidate&, const CandidateGenerator::Candidate&, QList<AngleInfo>, ThreadLocalData&);

  QAtomicInt candidatePos;
  CandidateGenerator candidates;

  bool shouldStop;

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

  bool symmetryEquivalentSolutionPresent(const Mat3D& R, int& checkFrom);
};

#include "indexing/solution.h"

#endif
