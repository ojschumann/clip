#include "ruleritem.h"
#include <QPainter>
#include <QCursor>
#include <iostream>

using namespace std;


RulerItem::RulerItem(const QPointF& p1, const QPointF& p2, qreal r, QGraphicsItem* parent):
    QGraphicsObject(parent),
    startHandle(new QGraphicsEllipseItem(this)),
    endHandle(new QGraphicsEllipseItem(this)),
    radius(0.01*r)
{
  startHandle->setPos(p1);
  endHandle->setPos(p2);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  QList<QGraphicsEllipseItem*> l;
  l << startHandle << endHandle;
  foreach (QGraphicsEllipseItem* item, l) {
    item->setRect(-radius, -radius, 2*radius, 2*radius);
    item->setPen(QPen(Qt::yellow));
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setCursor(QCursor(Qt::SizeAllCursor));
    //item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
  }
}


RulerItem::~RulerItem() {
}

QRectF RulerItem::boundingRect() const {
  QRectF r1 = startHandle->boundingRect().translated(startHandle->pos());
  QRectF r2 = endHandle->boundingRect().translated(endHandle->pos());
  return r1 | r2;
}

void RulerItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
  if (startHandle->pos()!=endHandle->pos()) {
    cout << "RulerItem " << startHandle->pos().x() << " " << startHandle->pos().y() << " <-> " << endHandle->pos().x() << " " << endHandle->pos().y() << endl;
    p->setPen(Qt::yellow);
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
  cout << "SetStart " << p.x() << " " << p.y() << endl;

  startHandle->setPos(p);
}

void RulerItem::setEnd(const QPointF& p) {
  cout << "SetEnd " << p.x() << " " << p.y() << endl;
  endHandle->setPos(p);
}

QPointF RulerItem::getStart() {
  return startHandle->pos();
}

QPointF RulerItem::getEnd() {
  return endHandle->pos();
}


QVariant RulerItem::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == ItemTransformChange) {
    startHandle->setTransform(value.value<QTransform>());
    endHandle->setTransform(value.value<QTransform>());
    return QVariant(transform());
  }
  return QGraphicsItem::itemChange(change, value);
}


