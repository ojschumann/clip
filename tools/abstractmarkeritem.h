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
  virtual Vec3D getMarkerNormal() const = 0;

  virtual Vec3D getRationalIndex();
  virtual TVec3D<int> getIntegerIndex();
  double getBestScore();

  void setIndex(const TVec3D<int>& index);

  MarkerType getType() const { return markerType; }
  virtual Vec3D normalToIndex(const Vec3D&)=0;

  void invalidateCache();

  virtual void highlight(bool b)=0;

protected:
  void calcBestIndex();
  MarkerType markerType;
  Vec3D rationalIndex;
  TVec3D<int> integerIndex;
  double deviation;
};

class AbstractProjectorMarkerItem: public AbstractMarkerItem
{
public:
  AbstractProjectorMarkerItem(Projector* p, MarkerType t);
  virtual ~AbstractProjectorMarkerItem();
  Projector* getProjector() const { return projector; }
protected:
  virtual Vec3D normalToIndex(const Vec3D&);
  Projector* projector;
};


#endif // ABSTRACTMARKERITEM_H
