#ifndef CANDIDATEGENERATOR_H
#define CANDIDATEGENERATOR_H

#include <QObject>
#include <QList>

#include "tools/vec3D.h"
#include "tools/mat3D.h"

class CandidateGenerator: public QObject
{
  Q_OBJECT
public:
  struct Candidate {
    TVec3D<int> index;
    Vec3D indexNormal;
    Vec3D realNormal;
    Vec3D reziprocalNormal;
  };

  CandidateGenerator(const Mat3D&, const Mat3D&, QObject* parent=0);
  Candidate getCandidate(int);

  void reset();
signals:
  void nextMajorIndex(int);
private:
  void generateNextIndex();
  void addToGroup(const TMat3D<int>&);

  QList< TMat3D<int> > group;
  QList< Candidate > candidates;
  TVec3D<int> maxIndex;

  Mat3D MReal;
  Mat3D MReciprocal;
};

#endif // CANDIDATEGENERATOR_H
