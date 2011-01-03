#include "solution.h"

#include "tools/optimalrotation.h"

Solution::Solution() {
  maxHklDeviation = -1.0;
}

Solution::Solution(const Solution &s) {
  *this = s;
}

Solution& Solution::operator=(const Solution& s) {
  items=s.items;
  indexingRotation=s.indexingRotation;
  bestRotation=s.bestRotation;
  maxHklDeviation = s.maxHklDeviation;
  return *this;
}


bool Solution::addMarkers(QList<Vec3D> markers, Indexer::MarkerType markerType, const Indexer::AngleInfo& a, const TVec3D<int>& index1, const TVec3D<int>& index2) {
  for (int n=0; n<markers.size(); n++) {
    if ((a.index1==n) && (a.type1==markerType)) {
      if (!addWithIndex(markers.at(n), markerType, index1)) return false;
    } else if ((a.index2==n) && (a.type2==markerType)) {
      if (!addWithIndex(markers.at(n), markerType, index2)) return false;
    } else {
      if (!tryToIndex(markers.at(n), markerType)) return false;
    }
  }
  return true;
}

bool Solution::tryToIndex(const Vec3D &marker, Indexer::MarkerType markerType) {
  Vec3D v = markerNormalToIndex(marker, markerType, indexingRotation);
  v.normalize();
  double preliminaryScale = 1.0/std::max(fabs(v.x()), std::max(fabs(v.y()), fabs(v.z())));

  for (int n=1; n<maxHKL; n++) {
    Vec3D hkl = v * preliminaryScale * n;
    for (int i=0; i<3; i++) hkl(i) = qRound(hkl(i));
    //0 = d/ds sum (s*v_i - hkl_i)^2
    // => 0 = sum (s*v_i - hkl_i) * v_i => s sum v_i^2 = sum v_i*hkl_i
    double scale = hkl*v;
    Vec3D rationalHkl = v*scale;
    double hklDeviation = (rationalHkl-hkl).norm();
    if (hklDeviation<=maxHklDeviation) {
      items << SolutionItem(hkl.toType<int>(), rationalHkl, marker, markerType);
      return true;
    }
  }
  return false;
}

bool Solution::addWithIndex(const Vec3D &marker, Indexer::MarkerType markerType, const TVec3D<int> &index) {
  Vec3D v = markerNormalToIndex(marker, markerType, indexingRotation);
  double scale = v*index.toType<double>()/v.norm_sq();
  double hklDeviation = (v*scale-index.toType<double>()).norm();
  if (hklDeviation>maxHklDeviation) return false;
  SolutionItem s(index, v*scale, marker, markerType);
  s.initialIndexed = true;
  items << s;
  return true;
}

Vec3D Solution::markerNormalToIndex(const Vec3D &marker, Indexer::MarkerType markerType, const Mat3D& rotation) {
  if (markerType==Indexer::Spot) {
    // marker = Rot*MReziprocal*hkl
    return MReziprocalInv*rotation*marker;
  } else {
    return MRealInv*rotation*marker;
  }
}

void Solution::calcBestRotation() {
  OptimalRotation opt;
  foreach (SolutionItem item, items) {
    Vec3D v;
    if (item.markerType==Indexer::Spot) {
      v = MReziprocal*item.hkl.toType<double>();
    } else {
      v = MReal*item.hkl.toType<double>();
    }
    opt.addVectorPair(v, item.markerNormal);
  }
  bestRotation = opt.getOptimalRotation();

  for (int n=0; n<items.size(); n++) {
    Vec3D v = markerNormalToIndex(items.at(n).markerNormal, items.at(n).markerType, bestRotation.transposed());
    v.normalize();
    items[n].rationalHkl = v * (v*items.at(n).hkl.toType<double>());
  }

}

double Solution::angularDeviationSum() const {
  double s=0.0;
  for (int n=0; n<items.size(); n++)
    s+=items.at(n).angularDeviation();
  return s;
}

double Solution::spatialDeviationSum() const {
  double s=0.0;
  foreach (SolutionItem item, items)
    s+=item.spatialDeviation();
  return s;
}



double Solution::hklDeviationSum() const {
  double s=0.0;
  foreach (SolutionItem item, items)
    s+=item.hklDeviation();
  return s;
}


SolutionItem::SolutionItem(const TVec3D<int> &_hkl, const Vec3D &_rationalHkl, const Vec3D &marker, Indexer::MarkerType type):
    hkl(_hkl),
    rationalHkl(_rationalHkl),
    markerNormal(marker),
    markerType(type)

{
  initialIndexed=false;
}

double SolutionItem::angularDeviation() const {
  return 0; //180.0*M_1_PI*fabs(acos(rotatedMarker*latticeVector));
}

double SolutionItem::spatialDeviation() const {
  return 0; // fabs((rotatedMarker-latticeVector).norm());
}

double SolutionItem::hklDeviation() const {
  return (rationalHkl-hkl.toType<double>()).norm();
}

int Solution_ID = qRegisterMetaType<Solution>("Solution");
