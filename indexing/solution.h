#ifndef SOLUTION_H
#define SOLUTION_H

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

Q_DECLARE_METATYPE(Solution)

#endif // SOLUTION_H
