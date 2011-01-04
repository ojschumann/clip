#include "zoneitem.h"
#include <QPainter>
#include <QPen>
#include <QCursor>
#include <iostream>
#include <QPointF>

#include "core/projector.h"
#include "tools/circleitem.h"
#include "tools/xmltools.h"

using namespace std;

ZoneItem::ZoneItem(const QPointF& p1, const QPointF& p2, Projector* p, QGraphicsItem* parent):
    PropagatingGraphicsObject(parent),
    AbstractProjectorMarkerItem(p, AbstractMarkerItem::ZoneMarker),
    imgRect(-0.01, -0.01, 1.02, 1.02),
    startHandle(new CircleItem(0.1, this)),
    endHandle(new CircleItem(0.1, this))
{
  highlight(false);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  QList<CircleItem*> l = QList<CircleItem*>() << startHandle << endHandle;
  connect(projector, SIGNAL(projectionParamsChanged()), this, SLOT(updatePolygon()));
  connect(projector, SIGNAL(projectionParamsChanged()), this, SLOT(updateOptimalZone()));
  foreach (CircleItem* item, l) {
    item->setRadius(projector->getSpotSize());
    item->setColor(Qt::red);
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setCursor(QCursor(Qt::SizeAllCursor));
    connect(item, SIGNAL(positionChanged()), this, SIGNAL(positionChanged()));
    connect(item, SIGNAL(positionChanged()), this, SLOT(updatePolygon()));
    connect(item, SIGNAL(positionChanged()), this, SLOT(updateOptimalZone()));
    connect(item, SIGNAL(itemClicked()), this, SIGNAL(itemClicked()));
    connect(projector, SIGNAL(spotSizeChanged(double)), item, SLOT(setRadius(double)));
  }
  updatePolygon();
  updateOptimalZone();
  startHandle->setPos(p1);
  endHandle->setPos(p2);
}


ZoneItem::~ZoneItem() {
}

QRectF ZoneItem::boundingRect() const {
  return imgRect;
}

QPainterPath ZoneItem::shape() const {
  QPainterPath path;
  foreach (QPolygonF poly, zonePolys)
    path.addPolygon(poly);
  return path;
}

void ZoneItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
  p->setPen(Qt::NoPen);
  p->setBrush(QBrush(QColor(255, 128, 0, 128)));
  foreach (QPolygonF poly, zonePolys)
    p->drawPolygon(poly);

  QPen pen;
  pen.setWidthF(highlighted?2.0:1.0);
  pen.setCosmetic(true);
  pen.setColor(Qt::black);
  pen.setStyle(Qt::DashLine);
  p->setPen(pen);
  foreach (QPolygonF poly, zoneLines)
    p->drawPolyline(poly);
}


bool PointSort(const QPointF& p1, const QPointF& p2) {
  return atan2(p1.y()-0.5, p1.x()-0.5)<atan2(p2.y()-0.5, p2.x()-0.5);
}

QPolygonF getPath(const QPointF& from, const QPointF& to, QRectF on, bool clockwise, QPointF& via) {
  QPolygonF path;
  path << from;
  int quadrant = int(M_2_PI*(atan2(from.y()-0.5, from.x()-0.5)+5*M_PI/4))%4;
  int quadrant_to = int(M_2_PI*(atan2(to.y()-0.5, to.x()-0.5)+5*M_PI/4))%4;

  //cout << "GetPath:" << quadrant << " " << quadrant_to << " ";
  //cout << M_2_PI*(atan2(from.y()-0.5, from.x()-0.5)+5*M_PI/4) << " ";
  //cout << M_2_PI*(atan2(to.y()-0.5, to.x()-0.5)+5*M_PI/4) << endl;

  QPolygonF corners;
  corners << on.topLeft() << on.topRight() << on.bottomRight() << on.bottomLeft();

  while (quadrant!=quadrant_to) {
    //cout << "Quadrant " << quadrant << endl;
    if (clockwise) {
      path << corners[quadrant];
      quadrant = (quadrant+1)%4;
    } else {
      quadrant = (quadrant+3)%4;
      path << corners[quadrant];
    }
  }

  path << to;

  via = 0.5*(path[0]+path[1]);

  return path;
}

void ZoneItem::updatePolygon() {
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

    //cout << "Generate Border Points" << endl;
    QList<QPointF> borderPoints;
    QRectF sImgRect = imgRect.adjusted(0.0001, 0.0001, -0.0001, -0.0001);
    foreach (QPolygonF q, polys) {
      if (!sImgRect.contains(q.first())) borderPoints << q.first();
      if (!sImgRect.contains(q.last())) borderPoints << q.last();
    }
    qSort(borderPoints.begin(), borderPoints.end(), PointSort);

    //cout << "Clear Border Points " << borderPoints.size() <<  endl;
    if (borderPoints.size()%2==1) {
      cout << "odd borderpointsize..." << endl;
      foreach (QPointF p, borderPoints) {
        cout << "(" << p.x() << "," << p.y() << ")" << endl;
      }
      cout << "Polys.size()=" << polys.size() << endl;
      foreach (QPolygonF p, polys) {
        cout << "(" << p.first().x() << "," << p.first().y() << ") -> ";
        cout << "(" << p.last().x() << "," << p.last().y() << ")" << endl;
      }
    }
    while (borderPoints.size()>1) {
      QPointF p = borderPoints.takeFirst();
      QPointF q;
      QPointF via;
      bool ok;
      QPolygonF cornerPath = getPath(p, borderPoints.first(), imgRect, true, via);
      Vec3D v = projector->det2normal(projector->img2det.map(via), ok);
      if (!ok || fabs(v*z)>sin(M_PI/180)) {
        cornerPath = getPath(p, borderPoints.last(), imgRect, false, via);
        q = borderPoints.takeLast();
      } else {
        q = borderPoints.takeFirst();
      }
      QPolygonF res;
      foreach (QPolygonF poly, polys) {
        if (poly.last()==p) {
          res << poly;
          polys.removeOne(poly);
          break;
        } else if (poly.first()==p) {
          for (int n=poly.size(); n--; ) res << poly[n];
          polys.removeOne(poly);
          break;
        }
      }
      res << cornerPath;
      if (res.first()!=q) {
        foreach (QPolygonF poly, polys) {
          if (poly.first()==q) {
            res << poly;
            polys.removeOne(poly);
            break;
          } else if (poly.last()==q) {
            for (int n=poly.size(); n--; ) res << poly[n];
            polys.removeOne(poly);
            break;
          }
        }
      }
      polys << res;

    }


    ////cout << "Search closed Items" << endl;
    QList<QPolygonF> closedItems;
    QPolygonF bigPoly;
    foreach (QPolygonF p, polys)
      if (p.isClosed()) {
      closedItems << p;
    } else {
      bigPoly << p;
    }
    if (!bigPoly.isEmpty())
      zonePolys << bigPoly;

    //cout << "Search containing Items" << endl;
    for (int n=0; n<closedItems.size(); n++) {
      for (int m=0; m<closedItems.size(); m++) {
        if (m==n) continue;
        bool allContained=true;
        foreach (QPointF p, closedItems[m]) {
          if (!closedItems[n].contains(p)) {
            allContained = false;
            break;
          }
        }
        if (allContained) {
          QPolygonF poly;
          closedItems[n] << closedItems[m];
          closedItems[m].clear();
        }
      }
    }
    //cout << "Fill items to zonePolys" << endl;
    foreach (QPolygonF p, closedItems) {
      if (!p.empty())
        zonePolys<< p;
    }

    //cout << "Finished" << endl;
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
void ZoneItem::updateOptimalZone() {
  Vec3D u = projector->det2normal(projector->img2det.map(startHandle->pos()));
  Vec3D v = projector->det2normal(projector->img2det.map(endHandle->pos()));
  // Vector perpendicular to u and v
  Vec3D z = u%v;
  z.normalize();

  Mat3D M;
  M.zero();
  M+= u^u;
  M+= v^v;

  foreach (v, projector->getSpotMarkerNormals()) {
    if (fabs(v*z)<sin(M_PI/180))
      M += v^v;
  }

  Mat3D Q1, Q2;
  M.svd(Q1, Q2);

  z = Q2.transposed()*Vec3D(0,0,1);
  if (z!=zoneNormal) {
    zoneNormal = z;
    emit positionChanged();
  }

  // Maximal scattering angle in this zone
  Vec3D n(-1,0,0);
  n = n-z*(z*n);
  if (n.norm()<1e-8) {
    n = Vec3D(0,0,1);
  } else {
    n.normalize();
  }

  zoneLines = generatePolygon(zoneNormal, n);
  update();

}





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
  bool ok=false;
  QPointF lastP;
  for (int i=0; i<401; i++) {
    QPointF p = projector->det2img.map(projector->normal2det(v, ok));
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
  if (firstOk && ok && !zonePoly.empty()) zonePoly << zonePolys.takeFirst();
  if (zonePoly.size()==400) zonePoly << zonePoly.first();
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

Vec3D ZoneItem::getMarkerNormal() const {
  return zoneNormal;
}

void ZoneItem::highlight(bool h) {
  highlighted = h;
  update();
}

const char XML_ZoneMarker_element[] = "Marker";
const char XML_ZoneMarker_start[] = "StartHandle";
const char XML_ZoneMarker_end[] = "EndHandle";

void ZoneItem::saveToXML(QDomElement base) {
  QDomElement marker = ensureElement(base, XML_ZoneMarker_element);
  PointToTag(marker, XML_ZoneMarker_start, getStart());
  PointToTag(marker, XML_ZoneMarker_end, getEnd());
}

void ZoneItem::loadFromXML(QDomElement base) {
  if (base.tagName()!=XML_ZoneMarker_element) return;
  for (QDomElement e=base.firstChildElement(); !e.isNull(); e=e.nextSiblingElement()) {
    if (e.tagName()==XML_ZoneMarker_start) {
      setStart(TagToPoint(e, getStart()));
    } else if (e.tagName()==XML_ZoneMarker_end) {
      setEnd(TagToPoint(e, getEnd()));
    }
  }
}
