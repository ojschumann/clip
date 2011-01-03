#include "spotitem.h"
#include "core/projector.h"

SpotItem::SpotItem(Projector *p, double r, QGraphicsItem *parent):
  CircleItem(r, parent),
  AbstractProjectorMarkerItem(p, AbstractMarkerItem::SpotMarker)
{

}


Vec3D SpotItem::getMarkerNormal() {
  return projector->det2normal(projector->img2det.map(pos()));
}
