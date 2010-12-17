#include "circleitem.h"
#include <QPainter>
#include <QPainterPath>


CircleItem::CircleItem(double r, QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
  skipNextPosChange=false;
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setRadius(r);
}

void CircleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  painter->setPen(QPen(color));
  painter->drawEllipse(QPointF(0,0), radius, radius);
}

QRectF CircleItem::boundingRect() const {
  return QRectF(-radius, -radius, 2*radius, 2*radius);
}

QPainterPath CircleItem::shape() const {
  QPainterPath path;
  path.addEllipse(QPointF(0,0), radius, radius);
  return path;
}

QVariant CircleItem::itemChange(GraphicsItemChange change, const QVariant & value ) {
  if (change==ItemPositionHasChanged) {
    if (not skipNextPosChange) {
      emit positionChanged();
    } else {
      skipNextPosChange=false;
    }
  }
  return QGraphicsObject::itemChange(change, value);
}

void CircleItem::setPosNoSig(const QPointF &p) {
  if (p!=pos()) {
    skipNextPosChange=true;
    setPos(p);
  }
}

void CircleItem::setRadius(double r) {
  radius=r;
  update();
}

void CircleItem::setColor(QColor c) {
  color = c;
  update();
}
