#ifndef __INDEXER_H__
#define __INDEXER_H__

#include <QRunnable>
#include <QMutex>
#include <QAtomicInt>
#include <QReadWriteLock>
#include <QTime>

#include "tools/vec3D.h"
#include "indexing/candidategenerator.h"

class Solution;

class Indexer: public QObject, public QRunnable {
  Q_OBJECT
public:
  enum MarkerType {
    Spot,
    Zone
  };
  class AngleInfo {
  public:
    AngleInfo(const Vec3D &_v1, MarkerType t1, int i1, const Vec3D &_v2, MarkerType t2, int i2, double maxDeviation);
    bool operator<(const AngleInfo &o) const;

    double lowerBound;
    double cosAng;
    double upperBound;

    Vec3D v1;
    MarkerType type1;
    int index1;

    Vec3D v2;
    MarkerType type2;
    int index2;
  };

  Indexer(QList<Vec3D> spotMarkerNormals, QList<Vec3D> zoneMarkerNormals, const Mat3D& MReal, const Mat3D& MReziprocal, double _maxAngularDeviation, double _maxHKLDeviation, int _maxHKL, QList< TMat3D<int> > _lauegroup);

  void run();

public slots:
  void stop();

signals:
  void publishSolution(Solution s);
  void progressInfo(int);
  void nextMajorIndex(int);

protected:
  void checkGuess(const CandidateGenerator::Candidate&, const CandidateGenerator::Candidate&, const AngleInfo &);
  void checkPossibleAngles(const CandidateGenerator::Candidate&, const CandidateGenerator::Candidate&, QList<AngleInfo>);

  QAtomicInt candidatePos;
  CandidateGenerator candidates;

  bool shouldStop;

  QList<AngleInfo> spotSpotAngles;
  QList<AngleInfo> spotZoneAngles;
  QList<AngleInfo> zoneZoneAngles;

  QList<Vec3D> spotMarkerNormals;
  QList<Vec3D> zoneMarkerNormals;
  Mat3D MReal;
  Mat3D MRealInv;
  Mat3D MReziprocal;
  Mat3D MReziprocalInv;

  double maxHKLDeviation;
  int maxHKL;

  struct SolutionInfo{
    Mat3D R;
    double score;
  };
  QList<SolutionInfo> uniqSolutions;
  QList<Mat3D> lauegroup;
  QReadWriteLock uniqLock;

  bool symmetryEquivalentSolutionPresent(const Mat3D& R, int& checkFrom);
};

#include "indexing/solution.h"

#endif
