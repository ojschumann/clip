#include "spotitem.h"
#include "core/projector.h"

SpotItem::SpotItem(Projector *p, double r, QGraphicsItem *parent):
  CircleItem(r, parent),
  AbstractProjectorMarkerItem(p, AbstractMarkerItem::SpotMarker)
{
  highlight(false);
}


Vec3D SpotItem::getMarkerNormal() const {
  return projector->det2normal(projector->img2det.map(pos()));
}

void SpotItem::highlight(bool b) {
  setLineWidth(b?2.0:1.0);
}
