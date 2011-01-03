#ifndef ABSTRACTMARKERITEM_H
#define ABSTRACTMARKERITEM_H

#include "tools/vec3D.h"

class Projector;

class AbstractMarkerItem
{
public:
  enum MarkerType {
    SpotMarker,
    ZoneMarker
  };

  AbstractMarkerItem(MarkerType t);
  virtual ~AbstractMarkerItem();
  virtual Vec3D getMarkerNormal()=0;
  virtual Vec3D getBestIndex();
  MarkerType getType() { return MarkerType; }
protected:
  virtual Vec3D normalToIndex(const Vec3D&)=0;
  MarkerType markerType;
};

class AbstractProjectorMarkerItem: public AbstractMarkerItem
{
public:
  AbstractProjectorMarkerItem(Projector* p, MarkerType t);
  virtual ~AbstractProjectorMarkerItem();
protected:
  virtual Vec3D normalToIndex(const Vec3D&);
  Projector* projector;
};


#endif // ABSTRACTMARKERITEM_H
