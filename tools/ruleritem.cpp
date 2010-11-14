#include "ruleritem.h"
#include <QPainter>
#include <QCursor>
#include <iostream>
#include <core/projector.h>
#include <tools/signalingellipse.h>

using namespace std;


RulerItem::RulerItem(const QPointF& p1, const QPointF& p2, Projector* p, QGraphicsItem* parent):
    PropagatingGraphicsObject(parent),
    startHandle(new SignalingEllipseItem(this)),
    endHandle(new SignalingEllipseItem(this)),
    projector(p)
{
  highlighted=true;
  highlight(false);
  startHandle->setPos(p1);
  endHandle->setPos(p2);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  QList<SignalingEllipseItem*> l;
  l << startHandle << endHandle;
  double radius = 0.01*projector->getSpotSize();
  foreach (SignalingEllipseItem* item, l) {
    item->setRect(-radius, -radius, 2*radius, 2*radius);
    item->setPen(QPen(Qt::red));
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
  double radius = 0.01*projector->getSpotSize();
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

    double radius = 0.01*projector->getSpotSize();

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
    startHandle->setPen(pen);
    endHandle->setPen(pen);

    update();
  }
}

bool RulerItem::isHighlighted() {
  return highlighted;
}


