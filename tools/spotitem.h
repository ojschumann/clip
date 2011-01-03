#ifndef SPOTITEM_H
#define SPOTITEM_H

#include "tools/circleitem.h"
#include "tools/abstractmarkeritem.h"

class SpotItem : public CircleItem, public AbstractProjectorMarkerItem
{
  Q_OBJECT
public:
  explicit SpotItem(Projector* p, double r, QGraphicsItem* parent = 0);
  virtual Vec3D getMarkerNormal();
};

#endif // SPOTITEM_H
