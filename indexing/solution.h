#ifndef SOLUTION_H
#define SOLUTION_H

#include <QList>

#include "tools/vec3D.h"
#include "tools/mat3D.h"

class SolutionItem {
public:
  int h;
  int k;
  int l;
  Vec3D rotatedMarker;
  Vec3D rationalHkl;
  Vec3D latticeVector;
  bool initialIndexed;
  double angularDeviation() const;
  double spatialDeviation() const;
  double hklDeviation() const;
};

class Solution {
public:
  Solution() {};
  Solution(const Solution& s) {
    items=s.items;
    indexingRotation=s.indexingRotation;
    bestRotation=s.bestRotation;
  };
  Solution operator=(const Solution& s) {
    items=s.items;
    indexingRotation=s.indexingRotation;
    bestRotation=s.bestRotation;
    return *this;
  };
  QList<SolutionItem> items;
  Mat3D indexingRotation;
  Mat3D bestRotation;
  double angularDeviationSum() const;
  double spatialDeviationSum() const;
  double hklDeviationSum() const;

};

#include <QMetaType>
Q_DECLARE_METATYPE(Solution);

#endif // SOLUTION_H
