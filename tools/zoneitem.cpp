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
    imgRect(0.01, 0.01, 0.98, 0.98),
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
  return imgRect;
}

void ZoneItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
  QPen pen;
  pen.setColor(QColor(255, 128, 0, 128));
  pen.setWidth(0);
  pen.setStyle(Qt::DashLine);
  p->setPen(pen);
  foreach (QPolygonF poly, zoneLines)
    p->drawPolyline(poly);

  //p->setPen(Qt::DashLine);

  //p->setBrush(QBrush(QColor(255, 128, 0, 128)));
  //foreach (QPolygonF poly, zonePolys)
  //  p->drawPolyline(poly);

  QList<QColor> c;
  c << Qt::red << Qt::blue << Qt::green << Qt::black;
  for (int i=0; i<zonePolys.size(); i++) {
    //p->setPen(c[i%4]);
    p->drawPolyline(zonePolys.at(i));
  }

}


bool PointSort(const QPointF& p1, const QPointF& p2) {
  return atan2(p1.y()-0.5, p1.x()-0.5)<atan2(p2.y()-0.5, p2.x()-0.5);
}

QPolygonF getPath(const QPointF& from, const QPointF& to, QRectF on, bool clockwise, QPointF& via) {
  QPolygonF path;
  path << from;
  int quadrant = int(M_2_PI*(atan2(from.y()-0.5, from.x()-0.5)+5*M_PI/4))%4;
  int quadrant_to = int(M_2_PI*(atan2(to.y()-0.5, to.x()-0.5)+5*M_PI/4))%4;

  cout << "GetPath:" << quadrant << " " << quadrant_to << " ";
  cout << M_2_PI*(atan2(from.y()-0.5, from.x()-0.5)+5*M_PI/4) << " ";
  cout << M_2_PI*(atan2(to.y()-0.5, to.x()-0.5)+5*M_PI/4) << endl;

  QPolygonF corners;
  corners << on.topLeft() << on.topRight() << on.bottomRight() << on.bottomLeft();

  while (quadrant!=quadrant_to) {
    path << corners[quadrant];
    if (clockwise) {
      quadrant = (quadrant+1)%4;
    } else {
      quadrant = (quadrant+3)%4;
    }
  }

  path << to;

  via = 0.5*(path[0]+path[1]);

  return path;
}

void ZoneItem::updatePolygon() {
  zoneLines.clear();
  zonePolys.clear();
  if (startHandle->pos()!=endHandle->pos()) {

    Vec3D u = projector->det2normal(projector->img2det.map(startHandle->pos()));
    Vec3D v = projector->det2normal(projector->img2det.map(endHandle->pos()));
    // Vector perpendicular to u and v
    Vec3D z = u%v;
    z.normalize();
    // Maximal scattering angle in this zone
    Vec3D n(-1,0,0);
    n = n-z*(z*n);
    if (n.norm()<1e-8) {
      n = Vec3D(0,0,1);
    } else {
      n.normalize();
    }

    // Rotate u 1 deg out of plane
    Mat3D R = Mat3D(z%n, M_PI*1.0/180);
    v = R*n;
    u = R.transposed()*n;

    QList<QPolygonF> polys;
    polys << generatePolygon(z*(-1), u) << generatePolygon(z, v);

    QList<QPointF> borderPoints;
    QRectF sImgRect = imgRect.adjusted(0.0001, 0.0001, -0.0001, -0.0001);
    foreach (QPolygonF q, polys) {
      if (!sImgRect.contains(q.first())) borderPoints << q.first();
      if (!sImgRect.contains(q.last())) borderPoints << q.last();
    }

    qSort(borderPoints.begin(), borderPoints.end(), PointSort);

    QPolygonF fillpoly;
    while (!polys.empty()) {
      int idx;
      if (fillpoly.empty() || ((idx=borderPoints.lastIndexOf(fillpoly.last()))==-1)) {
        fillpoly << polys.takeFirst();
      } else {
        // last point is on Border
        int idx2 = (idx+1)%borderPoints.size();
        QPointF via;
        bool ok;
        QPolygonF cornerPath = getPath(borderPoints[idx], borderPoints[idx2], imgRect, true, via);
        Vec3D v = projector->det2normal(projector->img2det.map(via), &ok);
        if (!ok || fabs(v*z)>sin(M_PI/180)) {
          idx2 = (idx+borderPoints.size()-1)%borderPoints.size();
          cornerPath = getPath(borderPoints[idx], borderPoints[idx2], imgRect, false, via);
        }
        for (int i=1; i<cornerPath.size()-1; i++) fillpoly << cornerPath.at(i);
        borderPoints.removeOne(cornerPath.first());
        borderPoints.removeOne(cornerPath.last());
        QPointF next = cornerPath.last();

        if (fillpoly.first()==next) {
          idx = idx2;
          idx2 = (idx+1)%borderPoints.size();
          cornerPath = getPath(borderPoints[idx], borderPoints[idx2], imgRect, true, via);
          v = projector->det2normal(projector->img2det.map(via), &ok);
          if (!ok || fabs(v*z)>sin(M_PI/180)) {
            idx2 = (idx+borderPoints.size()-1)%borderPoints.size();
            cornerPath = getPath(borderPoints[idx], borderPoints[idx2], imgRect, false, via);
          }
          for (int i=1; i<cornerPath.size(); i++) fillpoly << cornerPath.at(i);
          borderPoints.removeOne(cornerPath.first());
          borderPoints.removeOne(cornerPath.last());
          zonePolys << fillpoly;
          fillpoly.clear();
        } else {
          foreach (QPolygonF q, polys) {
            if (next==q.first()) {
              polys.removeOne(q);
              fillpoly << q;
              break;
            } else if (next==q.last()) {
              polys.removeOne(q);
              for (int i=q.size(); i--; ) fillpoly << q[i];
              break;
            }
          }
        }
      }
    }
    if (!fillpoly.empty())
      zonePolys << fillpoly;

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
  QList<QLineF> imgBorders;
  imgBorders << QLineF(imgRect.topLeft(), imgRect.topRight());
  imgBorders << QLineF(imgRect.topRight(), imgRect.bottomRight());
  imgBorders << QLineF(imgRect.bottomRight(), imgRect.bottomLeft());
  imgBorders << QLineF(imgRect.bottomLeft(), imgRect.topLeft());

  Vec3D v(_v);
  Mat3D M(n, 2.0*M_PI/400);
  QPolygonF zonePoly;
  QList<QPolygonF> zonePolys;

  bool firstOk = false;
  bool lastOk=false;
  QPointF lastP;
  for (int i=0; i<401; i++) {
    bool ok;
    QPointF p = projector->det2img.map(projector->normal2det(v, &ok));
    ok = ok && imgRect.contains(p);
    if (i>0) {
      if (lastOk != ok) {
        QPointF onBorder;
        foreach (QLineF border, imgBorders) {
          if (border.intersect(QLineF(p, lastP), &onBorder)==QLineF::BoundedIntersection) {
            zonePoly << onBorder;
            break;
          }
        }
      }
      if (ok) {
        zonePoly << p;
      } else if (zonePoly.size()>1) {
        zonePolys << zonePoly;
        zonePoly.clear();
      }
    }
    lastOk = ok;
    lastP = p;
    if (i==1) firstOk=ok;
    v = M*v;
  }
  if (zonePoly.size()>1) zonePolys << zonePoly;


  return zonePolys;
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


