#include "abstractmarkeritem.h"

#include "core/projector.h"
#include "core/crystal.h"

AbstractMarkerItem::AbstractMarkerItem(MarkerType t):
  markerType(t)
{
  deviation = -1;
}

AbstractMarkerItem::~AbstractMarkerItem() {}

Vec3D AbstractMarkerItem::getRationalIndex() {
  if (deviation<0) calcBestIndex();
  return rationalIndex;
}

TVec3D<int> AbstractMarkerItem::getIntegerIndex() {
  if (deviation<0) calcBestIndex();
  return integerIndex;
}

double AbstractMarkerItem::getBestScore() {
  if (deviation<0) calcBestIndex();
  return deviation;
}

void AbstractMarkerItem::calcBestIndex() {
  Vec3D v = normalToIndex(getMarkerNormal());
  v.normalize();
  double preliminaryScale = 1.0/std::max(fabs(v.x()), std::max(fabs(v.y()), fabs(v.z())));
  for (int n=1; n<20; n++) {
    Vec3D integerIdx = v * preliminaryScale * n;
    for (int i=0; i<3; i++) integerIdx(i) = qRound(integerIdx(i));
    //0 = d/ds sum (s*v_i - hkl_i)^2
    // => 0 = sum (s*v_i - hkl_i) * v_i => s sum v_i^2 = sum v_i*hkl_i
    double scale = integerIdx*v;
    Vec3D rationalIdx = v*scale;
    double testDeviation = (rationalIdx-integerIdx).norm();
    if (n==1 || testDeviation<deviation) {
      rationalIndex = rationalIdx;
      integerIndex = integerIdx.toType<int>();
      deviation = testDeviation;
    }
  }
}

void AbstractMarkerItem::setIndex(const TVec3D<int> &index) {
  Vec3D v = normalToIndex(getMarkerNormal());
  v.normalize();

  rationalIndex = v * ( v * index.toType<double>() );
  integerIndex = index;
  deviation = (rationalIndex-integerIndex.toType<double>()).norm();
}

void AbstractMarkerItem::invalidateCache() {
  deviation = -1;
}


AbstractProjectorMarkerItem::AbstractProjectorMarkerItem(Projector *p, MarkerType t):
  AbstractMarkerItem(t),
  projector(p)
{
}

AbstractProjectorMarkerItem::~AbstractProjectorMarkerItem() {}

Vec3D AbstractProjectorMarkerItem::normalToIndex(const Vec3D &v) {
  Vec3D n = projector->getCrystal()->getRotationMatrix().transposed() * v;
  if (markerType==SpotMarker) {
    // v = Rot * MRezi * hkl  => hkl = MRezi.inv * Rot.trans * v
    return projector->getCrystal()->getRealOrientationMatrix().transposed() * n;
  } else {
    return projector->getCrystal()->getReziprocalOrientationMatrix().transposed() * n;
  }
}
