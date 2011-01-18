#include "spotitem.h"

#include <QPainter>

#include "core/projector.h"

SpotItem::SpotItem(Projector *p, double r, QGraphicsItem *parent):
  CircleItem(r, parent),
  AbstractProjectorMarkerItem(p, AbstractMarkerItem::SpotMarker)
{
  highlight(false);
  connect(this, SIGNAL(positionChanged()), this, SLOT(slotInvalidateCache()));
}


Vec3D SpotItem::getMarkerNormal() const {
  return projector->det2normal(projector->img2det.map(pos()));
}

void SpotItem::highlight(bool b) {
  //setLineWidth(b?2.0:1.0);
  if (b!=isHighlighted) {
    setZValue(b ? 10 : 0);
    isHighlighted = b;
    prepareGeometryChange();
    update();
  }
}

void SpotItem::slotInvalidateCache() {
  invalidateCache();
}

void SpotItem::slotSetMaxSearchIndex(int n) {
  setMaxSearchIndex(n);
}

void SpotItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  if (isHighlighted) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(QColor(0xFF,0x33,0x33, 0x80))));
    painter->drawEllipse(QPointF(0,0), 2*radius, 2*radius);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);
    painter->drawEllipse(QPointF(0,0), radius, radius);
  } else {
    CircleItem::paint(painter, option, widget);
  }
}

QRectF SpotItem::boundingRect() const {
  if (isHighlighted) {
    return QRectF(-2*radius, -2*radius, 4*radius, 4*radius);
  } else {
    return CircleItem::boundingRect();
  }
}

QPainterPath SpotItem::shape() const {
  if (isHighlighted) {
    QPainterPath path;
    path.addEllipse(QPointF(0,0), 2*radius, 2*radius);
    return path;
  } else {
    return CircleItem::shape();
  }
}
