#include "zoneitem.h"
#include <QPainter>
#include <QCursor>
#include <iostream>
#include <core/projector.h>

using namespace std;


ZoneItem::ZoneItem(const QPointF& p1, const QPointF& p2, qreal r, Projector* p, QGraphicsItem* parent):
    QGraphicsObject(parent),
    startHandle(new QGraphicsEllipseItem(this)),
    endHandle(new QGraphicsEllipseItem(this)),
    radius(0.01*r),
    projector(p)
{
  highlight(false);
  startHandle->setPos(p1);
  endHandle->setPos(p2);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  QList<QGraphicsEllipseItem*> l;
  l << startHandle << endHandle;
  foreach (QGraphicsEllipseItem* item, l) {
    item->setRect(-radius, -radius, 2*radius, 2*radius);
    item->setPen(QPen(Qt::red));
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setCursor(QCursor(Qt::SizeAllCursor));
  }
}


ZoneItem::~ZoneItem() {
}

QRectF ZoneItem::boundingRect() const {
  return childrenBoundingRect();
}

void ZoneItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
  if (startHandle->pos()!=startPos || endHandle->pos()!=endPos) {
    startPos=startHandle->pos();
    endPos=endHandle->pos();
    emit rulerChanged();
  }
  if (startHandle->pos()!=endHandle->pos()) {
    p->setPen(pen);
    QVector<QLineF> lines;
    QLineF l(startHandle->pos(), endHandle->pos());

    QPolygonF zone;
    Vec3D u = projector->det2normal(projector->img2det.map(startPos));
    Vec3D v = projector->det2normal(projector->img2det.map(endPos));
    Vec3D z = u%v;
    z.normalize();
    Vec3D n = u;
    Mat3D M(z, 2.0*M_PI/400);

    for (int i=0; i<401; i++) {
      bool ok;
      QPointF p2 = projector->det2img.map(projector->normal2det(n, &ok));
      if (ok) {
        zone << p2;
      } else if (zone.size()>1) {
        p->drawPolyline(zone);
        zone.clear();
      }
      n = M*n;
    }
    if (zone.size()>1) {
      p->drawPolyline(zone);
    }
  }
}

void ZoneItem::setStart(const QPointF& p) {
  startHandle->setPos(p);
}

void ZoneItem::setEnd(const QPointF& p) {
  endHandle->setPos(p);
}

QPointF ZoneItem::getStart() {
  return startHandle->pos();
}

QPointF ZoneItem::getEnd() {
  return endHandle->pos();
}

void ZoneItem::highlight(bool h) {
  if (h!=highlighted) {
    highlighted=h;
    if (isHighlighted()) {
      pen = QPen(QColor(255, 128, 0));
      pen.setWidthF(3.0);
      pen.setCosmetic(true);
    } else {
      //pen = QPen(Qt::yellow);
      //pen.setWidthF(0.0);
      pen = QPen(QColor(255, 255, 0, 128));
      pen.setWidthF(1.5*radius);
    }
    startHandle->setPen(pen);
    endHandle->setPen(pen);

    update();
  }
}

bool ZoneItem::isHighlighted() {
  return highlighted;
}

QVariant ZoneItem::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == ItemTransformChange) {
    startHandle->setTransform(value.value<QTransform>());
    endHandle->setTransform(value.value<QTransform>());
    return QVariant(transform());
  }
  return QGraphicsItem::itemChange(change, value);
}


