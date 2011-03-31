#include "spotitem.h"

#include <QPainter>

#include "core/projector.h"
#include "config/configstore.h"

SpotItem::SpotItem(Projector *p, double r, QGraphicsItem *parent):
  CircleItem(r, parent),
  AbstractProjectorMarkerItem(p, AbstractMarkerItem::SpotMarker)
{
  highlight(false);
  connect(this, SIGNAL(positionChanged()), this, SLOT(slotInvalidateCache()));

  setFlag(QGraphicsItem::ItemIsMovable, true);
  setCursor(QCursor(Qt::SizeAllCursor));
  ConfigStore::getInstance()->ensureColor(ConfigStore::SpotMarker, this, SLOT(setColor(QColor)));
  ConfigStore::getInstance()->ensureColor(ConfigStore::SpotMarkerHighlight, this, SLOT(doUpdate()));
  ConfigStore::getInstance()->ensureColor(ConfigStore::SpotMarkerHighlightBg, this, SLOT(doUpdate()));
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
    ConfigStore* config = ConfigStore::getInstance();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(config->color(ConfigStore::SpotMarkerHighlightBg)));
    painter->drawEllipse(QPointF(0,0), 2*radius, 2*radius);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(config->color(ConfigStore::SpotMarkerHighlight));
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
