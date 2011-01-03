#include "marker.h"

Marker::Marker(const Vec3D &n, MarkerType t):
    AbstractMarkerItem(t),
    markerNormal(n)
{
}


Vec3D Marker::getMarkerNormal() {
  return markerNormal;
}

Vec3D Marker::normalToIndex(const Vec3D &n) {
  return n;
}
