#ifndef SOLUTION_H
#define SOLUTION_H

#include <QList>

#include "tools/vec3D.h"
#include "tools/mat3D.h"
#include "indexing/indexer.h"

class SolutionItem {
public:
  SolutionItem(const TVec3D<int>&, const Vec3D&, const Vec3D& marker, Indexer::MarkerType type);
  TVec3D<int> hkl;
  Vec3D rationalHkl;
  Vec3D markerNormal;
  Indexer::MarkerType markerType;
  bool initialIndexed;
  double angularDeviation() const;
  double spatialDeviation() const;
  double hklDeviation() const;
};

class Solution {
public:
  Solution();
  Solution(const Solution& s);
  Solution& operator=(const Solution& s);

  bool addMarkers(QList<Vec3D>, Indexer::MarkerType, const Indexer::AngleInfo&, const TVec3D<int>&, const TVec3D<int>&);
  bool tryToIndex(const Vec3D& marker, Indexer::MarkerType markerType);
  bool addWithIndex(const Vec3D& marker, Indexer::MarkerType markerType, const TVec3D<int>& index);

  void calcBestRotation();

  QList<SolutionItem> items;
  Mat3D MReal;
  Mat3D MRealInv;
  Mat3D MReziprocal;
  Mat3D MReziprocalInv;
  Mat3D indexingRotation;
  Mat3D bestRotation;
  double angularDeviationSum() const;
  double spatialDeviationSum() const;
  double hklDeviationSum() const;

  double maxHklDeviation;
  double maxHKL;

  Vec3D markerNormalToIndex(const Vec3D& marker, Indexer::MarkerType markerType);

};

#include <QMetaType>
Q_DECLARE_METATYPE(Solution);

#endif // SOLUTION_H
