#ifndef MARKER_H
#define MARKER_H

#include "tools/abstractmarkeritem.h"
#include "tools/vec3D.h"
#include "tools/mat3D.h"

class Marker : public AbstractMarkerItem
{
public:
  Marker(const Vec3D& n, MarkerType t);
  Vec3D getMarkerNormal();
protected:
  Vec3D normalToIndex(const Vec3D &);
  Vec3D markerNormal;
};

#endif // MARKER_H
