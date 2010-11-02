#include "zoneitem.h"
#include <QPainter>
#include <QCursor>
#include <iostream>
#include <core/projector.h>
#include <tools/signalingellipse.h>

using namespace std;


ZoneItem::ZoneItem(const QPointF& p1, const QPointF& p2, Projector* p, QGraphicsItem* parent):
    QGraphicsObject(parent),
    startHandle(new SignalingEllipseItem(this)),
    endHandle(new SignalingEllipseItem(this)),
    projector(p)
{
  highlighted=true;
  highlight(false);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  QList<SignalingEllipseItem*> l;
  l << startHandle << endHandle;
  double radius = 0.01*projector->getSpotSize();
  connect(projector, SIGNAL(projectionParamsChanged()), this, SLOT(updatePolygon()));
  foreach (SignalingEllipseItem* item, l) {
    item->setRect(-radius, -radius, 2*radius, 2*radius);
    item->setPen(QPen(Qt::red));
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setCursor(QCursor(Qt::SizeAllCursor));
    connect(item, SIGNAL(positionChanged()), this, SIGNAL(zoneChanged()));
    connect(item, SIGNAL(positionChanged()), this, SLOT(updatePolygon()));
  }
  startHandle->setPos(p1);
  endHandle->setPos(p2);
}


ZoneItem::~ZoneItem() {
}

QRectF ZoneItem::boundingRect() const {
  QRectF r = childrenBoundingRect();
  foreach (QPolygonF zoneSeg, zoneSegments) {
    QRectF b = zoneSeg.boundingRect();
    double radius = 0.01*projector->getSpotSize();
    b.adjust(-radius, -radius, radius, radius);
    r |= b;
  }
  r &= projector->getScene()->sceneRect();
  return r;
}

void ZoneItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
  p->setPen(pen);
  foreach (QPolygonF poly, zoneSegments)
    p->drawPolyline(poly);
}

void ZoneItem::updatePolygon() {
  zoneSegments.clear();
  if (startHandle->pos()!=endHandle->pos()) {

    Vec3D u = projector->det2normal(projector->img2det.map(startHandle->pos()));
    Vec3D v = projector->det2normal(projector->img2det.map(endHandle->pos()));
    Vec3D z = u%v;
    z.normalize();
    Vec3D n = u;
    Mat3D M(z, 2.0*M_PI/400);

    QVector<bool> ok(400);
    QPolygonF zone(400);
    for (int i=0; i<400; i++) {
      zone[i] = projector->det2img.map(projector->normal2det(n, &ok[i]));
      n = M*n;
    }
    int startIdx = ok.indexOf(false);
    if (startIdx==-1) {
      zone << zone.first();
      zoneSegments << zone;
    } else if (ok.indexOf(true)!=-1) {
      int idx=(startIdx+1)%400;
      while (idx!=startIdx) {
        while (!ok[idx]) idx=(idx+1)%400;
        QPolygonF zoneSegment;
        while (ok[idx]) {
          zoneSegment << zone[idx];
          idx=(idx+1)%400;
        }
        if (zoneSegment.size()>1)
          zoneSegments << zoneSegment;
      }

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
  if (h!=isHighlighted()) {
    highlighted=h;
    double radius = 0.01*projector->getSpotSize();
    if (isHighlighted()) {
      pen = QPen(QColor(255, 192, 0, 128));
      pen.setWidthF(1.5*radius);
    } else {
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


