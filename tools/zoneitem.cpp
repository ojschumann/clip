#include "zoneitem.h"
#include <QPainter>
#include <QPen>
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
  return projector->getScene()->sceneRect();
}

void ZoneItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
  QPen pen;
  pen.setColor(QColor(255, 128, 0, 128));
  pen.setWidth(0);
  pen.setStyle(Qt::DashLine);
  p->setPen(pen);
  foreach (QPolygonF poly, zoneLines)
    p->drawPolyline(poly);

  p->setPen(Qt::NoPen);
  p->setBrush(QBrush(QColor(255, 128, 0, 128)));
  foreach (QPolygonF poly, zonePolys)
    p->drawPolygon(poly);
}


void ZoneItem::updatePolygon() {
  zoneLines.clear();
  zonePolys.clear();
  if (startHandle->pos()!=endHandle->pos()) {
    Vec3D u = projector->det2normal(projector->img2det.map(startHandle->pos()));
    Vec3D v = projector->det2normal(projector->img2det.map(endHandle->pos()));
    // Vector perpendicular to u and v
    Vec3D n = u%v;
    n.normalize();
    // in-plane Vector perpendiculat to u
    Vec3D r = n%u;
    // Rotate u 1 deg out of plane
    Mat3D R = Mat3D(r, M_PI*1.0/180);
    v = R*u;
    u = R.transposed()*u;

    QPolygonF poly;
    foreach (QPolygonF p, generatePolygon(n*(-1), u)) {
     cout << "(" << p.first().x() << "," << p.first().y() << ")-(" << p.last().x() << "," << p.last().y() << ") ";
      poly << p;
    }
    foreach (QPolygonF p, generatePolygon(n, v)) { poly << p;
    cout << "(" << p.first().x() << "," << p.first().y() << ")-(" << p.last().x() << "," << p.last().y() << ") ";
  }
    cout << endl << endl;
    zonePolys << poly;

  }
}

/*
 Optimal Plane:
 Set of Vectors vi
 M = sum(vi^vi)
 M.svd(Q1,Q2)
 smalest eigenvalue at M(2,2), thus Eigenvector with that value is
 n = Q2.transposed()*Vec3D(0,0,1)
 */
void ZoneItem::updateOptimalZone() {}

QList<QPolygonF> ZoneItem::generatePolygon(const Vec3D& n, const Vec3D& _v) {
  QList<QPolygonF> zoneSegs;
  Vec3D v(_v);
  Mat3D M(n, 2.0*M_PI/400);
  QPolygonF zone;
  bool first_ok;
  bool ok;
  for (int i=0; i<400; i++) {
    QPointF p = projector->det2img.map(projector->normal2det(v, &ok));
    if (ok) {
      zone << p;
    } else {
      if (zone.size()>1)
        zoneSegs << zone;
      zone.clear();
    }
    if (i==0) first_ok = ok;
    v = M*v;
  }
  if (first_ok && ok && !zoneSegs.empty()) {
    zone << zoneSegs.first();
    zoneSegs.removeFirst();
  }
  if (zone.size()==400) zone << zone.first();
  if (zone.size()>1)
    zoneSegs << zone;

  return zoneSegs;
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
      pen = QPen(QColor(0,0,0,255));
      pen.setWidthF(1.5*radius);
      pen.setWidthF(0);
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


