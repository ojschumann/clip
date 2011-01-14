#include "marker.h"
#include "indexer.h"

Marker::Marker(const Vec3D &n, MarkerType t, int _maxSearchIndex):
    AbstractMarkerItem(t),
    markerNormal(n),
    normalToIndexMatrix(0)
{
  setMaxSearchIndex(_maxSearchIndex);
}


Vec3D Marker::getMarkerNormal() const {
  return markerNormal;
}

Vec3D Marker::normalToIndex(const Vec3D &n) {
  return *normalToIndexMatrix * n;
}

Vec3D Marker::getIndexNormal() {
  Vec3D v = getIntegerIndex().toType<double>();
  v = *indexToNormalMatrix * v;
  v.normalize();
  return v;
  return (*indexToNormalMatrix * getIntegerIndex().toType<double>()).normalized();
}

void Marker::setMatrices(const Mat3D &SpotNormalToIndex, const Mat3D &ZoneNormalToIndex, const Mat3D &MReciprocal, const Mat3D &MReal) {
  if (markerType==SpotMarker) {
    normalToIndexMatrix = &SpotNormalToIndex;
    indexToNormalMatrix = &MReciprocal;
  } else {
    normalToIndexMatrix = &ZoneNormalToIndex;
    indexToNormalMatrix = &MReal;
  }
}
