#include "cropmarker.h"
#include "tools/signalingellipse.h"
#include <QPainter>
#include <QCursor>
#include <cmath>
#include "core/projector.h"

CropMarker::CropMarker(Projector* p, const QPointF &pCenter, double _dx, double _dy, double _angle, QGraphicsItem *parent):
    QGraphicsObject(parent),
    projector(p)
{

  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  double radius = 0.01*projector->getSpotSize();
  for (int i=0; i<4; i++) {
    SignalingEllipseItem* item = new SignalingEllipseItem(this);
    item->setRect(-radius, -radius, 2*radius, 2*radius);
    item->setPen(QPen(Qt::black));
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setCursor(QCursor(Qt::SizeAllCursor));
    handles << item;
  }

  handles[0]->setPos(0.1, 0.1);
  handles[1]->setPos(0.1, 0.9);
  handles[2]->setPos(0.9, 0.9);
  handles[3]->setPos(0.9, 0.1);

}

/*void CropMarker::setOuterHandlesFromData() {
  double s = sin(angle);
  double c = cos(angle);

  QPointF cp = centralHandle->pos();
  QPointF px = QPointF( c, s);
  QPointF py = QPointF( s,-c);

  outerHandleX->setPosNoSig(cp+dx*px);
  outerHandleY->setPosNoSig(cp+dy*py);
}

void CropMarker::centralHandleMoved() {
  setOuterHandlesFromData();
}

void CropMarker::outerHandleXMoved() {
  double _dx = outerHandleX->x()-centralHandle->x();
  double _dy = outerHandleX->y()-centralHandle->y();

  dx = hypot(_dx, _dy);
  angle = atan2(_dy, _dx);
  setOuterHandlesFromData();
}

void CropMarker::outerHandleYMoved() {
  double _dx = outerHandleY->x()-centralHandle->x();
  double _dy = outerHandleY->y()-centralHandle->y();

  dy = hypot(_dx, _dy);
  angle = atan2(_dx, -_dy);
  setOuterHandlesFromData();
}*/

#include "tools/debug.h"

QPolygonF CropMarker::getRect() {
  QPolygonF rect;
  foreach (SignalingEllipseItem* item, handles) {
    rect << item->pos();
  }
return rect;
}

void CropMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  QPolygonF rect = getRect();
  rect << rect.first();
  QPen pen = painter->pen();
  pen.setStyle(Qt::DashLine);
  painter->setPen(pen);
  painter->drawPolyline(rect);

}

QRectF CropMarker::boundingRect() const {
  return childrenBoundingRect();
}

QVariant CropMarker::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == ItemTransformChange) {
    foreach (SignalingEllipseItem* item, handles)
      item->setTransform(value.value<QTransform>());
    return QVariant(transform());
  }

  return QGraphicsItem::itemChange(change, value);
}
