#include "ruleritem.h"
#include <QPainter>
#include <QCursor>
#include <iostream>
#include "tools/circleitem.h"

using namespace std;


RulerItem::RulerItem(const QPointF& p1, const QPointF& p2, double r, QGraphicsItem* parent):
    PropagatingGraphicsObject(parent),
    startHandle(new CircleItem(r, this)),
    endHandle(new CircleItem(r,this)),
    radius(r)
{
  highlighted=true;
  highlight(false);
  startHandle->setPos(p1);
  endHandle->setPos(p2);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  QList<CircleItem*> l;
  l << startHandle << endHandle;
  foreach (CircleItem* item, l) {
    item->setColor(Qt::red);
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setCursor(QCursor(Qt::SizeAllCursor));
    connect(item, SIGNAL(positionChanged()), this, SIGNAL(rulerChanged()));
  }
}

RulerItem::~RulerItem() {
}

QRectF RulerItem::boundingRect() const {
  return childrenBoundingRect();
}

QPainterPath RulerItem::shape() const {
  QPainterPath path(startHandle->pos());
  path.addEllipse(startHandle->pos(), radius, radius);
  path.lineTo(endHandle->pos());
  path.addEllipse(endHandle->pos(), radius, radius);
  return path;
}

void RulerItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
  if (startHandle->pos()!=endHandle->pos()) {
    p->setPen(pen);
    QVector<QLineF> lines;
    QLineF l(startHandle->pos(), endHandle->pos());


    lines << l;
    l.setLength(0.7*radius);
    l.setAngle(l.angle()+30.0);
    lines << l;
    l.setAngle(l.angle()-60.0);
    lines << l;
    l = QLineF(endHandle->pos(), startHandle->pos());
    l.setLength(0.7*radius);
    l.setAngle(l.angle()+30.0);
    lines << l;
    l.setAngle(l.angle()-60.0);
    lines << l;
    p->drawLines(lines);
  }
}

void RulerItem::setStart(const QPointF& p) {
  startHandle->setPos(p);
}

void RulerItem::setEnd(const QPointF& p) {
  endHandle->setPos(p);
}

QPointF RulerItem::getStart() {
  return startHandle->pos();
}

QPointF RulerItem::getEnd() {
  return endHandle->pos();
}

void RulerItem::highlight(bool h) {
  if (h!=highlighted) {
    highlighted=h;
    if (isHighlighted()) {
      pen = QPen(QColor(255, 128, 0));
      pen.setWidthF(2.5);
      pen.setCosmetic(true);
    } else {
      pen = QPen(QColor(255, 128, 0));
      pen.setWidthF(0.0);
    }
    //startHandle->setPen(pen);
    //endHandle->setPen(pen);

    update();
  }
}

bool RulerItem::isHighlighted() {
  return highlighted;
}

void RulerItem::setHandleSize(double r) {
  radius = r;
  startHandle->setRadius(radius);
  endHandle->setRadius(radius);
}
