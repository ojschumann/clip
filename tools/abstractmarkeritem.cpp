#include "abstractmarkeritem.h"

#include "core/projector.h"
#include "core/crystal.h"

AbstractMarkerItem::AbstractMarkerItem(MarkerType t):
  markerType(t)
{
}

AbstractMarkerItem::~AbstractMarkerItem() {}

Vec3D AbstractMarkerItem::getBestIndex() {
  // TODO: Scale with n
  Vec3D v = normalToIndex(getMarkerNormal());
  v.normalize();
  double preliminaryScale = 1.0/std::max(fabs(v.x()), std::max(fabs(v.y()), fabs(v.z())));
  Vec3D bestHKL;
  double minDeviation;
  for (int n=1; n<20; n++) {
    Vec3D hkl = v * preliminaryScale * n;
    for (int i=0; i<3; i++) hkl(i) = qRound(hkl(i));
    //0 = d/ds sum (s*v_i - hkl_i)^2
    // => 0 = sum (s*v_i - hkl_i) * v_i => s sum v_i^2 = sum v_i*hkl_i
    double scale = hkl*v;
    Vec3D rationalHkl = v*scale;
    double hklDeviation = (rationalHkl-hkl).norm();
    if (n==1 || hklDeviation<minDeviation) {
      bestHKL = rationalHkl;
      minDeviation = hklDeviation;
    }
  }
  return bestHKL;
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
