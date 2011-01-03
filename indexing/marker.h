#ifndef MARKER_H
#define MARKER_H

#include "tools/abstractmarkeritem.h"
#include "tools/vec3D.h"
#include "tools/mat3D.h"


class Marker : public AbstractMarkerItem
{
public:
  Marker(const Vec3D& n, MarkerType t);
  Vec3D getMarkerNormal() const;
  Vec3D getIndexNormal();
  void setMatrices(const Mat3D& SpotNormalToIndex, const Mat3D& ZoneNormalToIndex, const Mat3D& MReciprocal, const Mat3D& MReal);
protected:
  Vec3D normalToIndex(const Vec3D &);
  Vec3D markerNormal;
  Mat3D const* normalToIndexMatrix;
  Mat3D const* indexToNormalMatrix;
  virtual void highlight(bool b) {}
};

#endif // MARKER_H
