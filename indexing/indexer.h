#ifndef __INDEXER_H__
#define __INDEXER_H__

#include <QRunnable>
#include <QMutex>
#include <QAtomicInt>

#include "tools/vec3D.h"
#include "indexing/solution.h"
#include "indexing/candidategenerator.h"

class Indexer: public QObject, public QRunnable {
  Q_OBJECT
public:
  Indexer(QList<Vec3D> spotMarkerNormals, QList<Vec3D> zoneMarkerNormals, const Mat3D& MReal, const Mat3D& MReziprocal);

  void run();

  void checkGuess(const CandidateGenerator::Candidate &c1, const CandidateGenerator::Candidate &c2,  const IndexWorker::AngleInfo &a);
  void otimizeScale(SolutionItem& si);

public slots:
  void stop();

signals:
  void publishSolution(Solution s);
  void progressInfo(int, int);

protected:
  class AngleInfo {
  public:
    enum NormalType {
      Spot,
      Zone
    };
    AngleInfo(const Vec3D& v1, const Vec3D& v2, NormalType t1, NormalType t2);
    bool operator<(const AngleInfo &o) const;
    static bool cmpAngleInfoLowerBound(const AngleInfo &a1, const AngleInfo &a2);
    static bool cmpAngleInfoUpperBound(const AngleInfo &a1, const AngleInfo &a2);
    int index1;
    int index2;
    int type1;
    int type2;
    double lowerBound;
    double cosAng;
    double upperBound;
  };

  bool newSolution(const Mat3D& M);

  QAtomicInt candidatePos;
  CandidateGenerator candidates;

  int nextProgressSignal;
  bool shouldStop;

  QList<AngleInfo> angles;

  QList<Vec3D> spotMarkerNormals;
  QList<Vec3D> zoneMarkerNormals;
  Mat3D MReal;
  Mat3D MReziprocal;

};

#endif
