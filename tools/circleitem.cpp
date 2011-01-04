#include "circleitem.h"
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>


CircleItem::CircleItem(double r, QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
  skipNextPosChange=false;
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setRadius(r);
  lineWidth = 1.0;
}

void CircleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  QPen pen(color);
  pen.setWidthF(lineWidth);
  pen.setCosmetic(true);
  painter->setPen(pen);
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

void CircleItem::setLineWidth(double lw) {
  lineWidth = lw;
  update();
}

#include <iostream>
using namespace std;
void CircleItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  cout << "Clicked" << endl;
  if (event->button()==Qt::LeftButton) emit itemClicked();
  QGraphicsObject::mousePressEvent(event);
}
