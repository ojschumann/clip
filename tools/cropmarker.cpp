#include "cropmarker.h"
#include "tools/circleitem.h"
#include <QPainter>
#include <QCursor>
#include <cmath>

CropMarker::CropMarker(const QPointF &pCenter, double _dx, double _dy, double _angle, double handleSize, QGraphicsItem *parent):
    PropagatingGraphicsObject(parent)
{

  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setHandleSize(handleSize);
  for (int i=0; i<4; i++) {
    CircleItem* item = new CircleItem(handleSize, this);
    item->setColor(Qt::black);
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setCursor(QCursor(Qt::SizeAllCursor));
    connect(item, SIGNAL(positionChanged()), &handleMapper, SLOT(map()));
    handleMapper.setMapping(item, i);
    handles << item;
  }

  handles[0]->setPos(0.1, 0.1);
  handles[1]->setPos(0.9, 0.1);
  handles[2]->setPos(0.9, 0.9);
  handles[3]->setPos(0.1, 0.9);

  connect(&handleMapper, SIGNAL(mapped(int)), this, SLOT(resizeMarkerMoved(int)));

  rotateMarker = new CircleItem(handleSize, this);
  rotateMarker->setColor(Qt::black);
  rotateMarker->setFlag(QGraphicsItem::ItemIsMovable);
  rotateMarker->setCursor(QCursor(Qt::ClosedHandCursor));
  rotateMarker->setPos(0.5, 0.1);


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
  foreach (CircleItem* item, handles) {
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

void CropMarker::setHandleSize(double r) {
  foreach (CircleItem* item, handles)
    item->setRadius(r);
}
