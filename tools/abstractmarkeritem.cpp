#include "abstractmarkeritem.h"

#include "core/projector.h"
#include "core/crystal.h"

AbstractMarkerItem::AbstractMarkerItem(MarkerType t):
  markerType(t)
{
  indexDeviation = -1;
}

AbstractMarkerItem::~AbstractMarkerItem() {}

Vec3D AbstractMarkerItem::getRationalIndex() {
  if (indexDeviation<0) calcBestIndex();
  return rationalIndex;
}

TVec3D<int> AbstractMarkerItem::getIntegerIndex() {
  if (indexDeviation<0) calcBestIndex();
  return integerIndex;
}

double AbstractMarkerItem::getIndexDeviationScore() {
  if (indexDeviation<0) calcBestIndex();
  return indexDeviation;
}

double AbstractMarkerItem::getDetectorPositionScore() {
  if (detectorPositionDeviation<0) calcDetectorDeviation();
  return detectorPositionDeviation;
}

double AbstractMarkerItem::getAngularDeviation() {
  if (angularDeviation<0) calcAngularDeviation();
  return angularDeviation;
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
    if (n==1 || testDeviation<indexDeviation) {
      rationalIndex = rationalIdx;
      integerIndex = integerIdx.toType<int>();
      indexDeviation = testDeviation;
    }
  }
}

void AbstractMarkerItem::calcDetectorDeviation() {
  detectorPositionDeviation = 0.0;
}

void AbstractMarkerItem::calcAngularDeviation() {
  angularDeviation = 0.0;
}

void AbstractMarkerItem::setIndex(const TVec3D<int> &index) {
  Vec3D v = normalToIndex(getMarkerNormal());
  v.normalize();

  rationalIndex = v * ( v * index.toType<double>() );
  integerIndex = index;
  indexDeviation = (rationalIndex-integerIndex.toType<double>()).norm();
}

void AbstractMarkerItem::invalidateCache() {
  indexDeviation = -1.0;
  angularDeviation = -1.0;
  detectorPositionDeviation = -1.0;
}





// #################################################

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

void AbstractProjectorMarkerItem::calcDetectorDeviation() {
  if (markerType==SpotMarker) {
    bool ok1, ok2;
    QPointF p = projector->normal2det(getMarkerNormal(), ok1);
    p -= projector->normal2det(projector->getCrystal()->hkl2Reziprocal(getIntegerIndex().toType<double>()).normalized(), ok2);
    if (ok1 && ok2) {
      detectorPositionDeviation = hypot(p.x(), p.y());
    }
  } else {
    double score = 0.0;
    int N = 0;
    Vec3D n = getMarkerNormal();
    QRectF plane(0, 0, 1, 1);
    foreach (Reflection r, projector->getProjectedReflectionsNormalToZone(getIntegerIndex())) {
      bool ok;
      QPointF pSpot = projector->normal2det(r.normal, ok);
      if (!ok || !plane.contains(projector->det2img.map(pSpot)))
        continue;
      Vec3D v = r.normal - n*(n*r.normal);
      v.normalize();
      QPointF pZone = projector->normal2det(v, ok);
      if (ok) {
        QPointF dp = pSpot - pZone;
        score += hypot(dp.x(), dp.y());
        N++;
      }
    }
    if (N>0) {
      detectorPositionDeviation = score/N;
    } else {
      detectorPositionDeviation = 0.0;
    }
  }
}

void AbstractProjectorMarkerItem::calcAngularDeviation() {
  Vec3D n;
  if (markerType==SpotMarker) {
    n = projector->getCrystal()->hkl2Reziprocal(getIntegerIndex().toType<double>());
  } else {
    n = projector->getCrystal()->uvw2Real(getIntegerIndex().toType<double>());
  }
  angularDeviation = 180.0*M_1_PI*acos(n.normalized()*getMarkerNormal());
}

