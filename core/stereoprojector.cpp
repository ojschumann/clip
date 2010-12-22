#include "core/stereoprojector.h"

#include <cmath>
#include <QtGui/QGraphicsEllipseItem>
#include <iostream>

#include "ui/stereocfg.h"
#include "tools/vec3D.h"
#include "core/reflection.h"
#include "core/projectorfactory.h"

using namespace std;

StereoProjector::StereoProjector(QObject* parent):
    Projector(parent),
    localCoordinates() {
  internalSetWavevectors(0, M_PI);
  scene.setSceneRect(QRectF(-1.0, -1.0, 2.0, 2.0));
  connect(this, SIGNAL(textSizeChanged(double)), this, SLOT(decorateScene()));
}

Projector* StereoProjector::getInstance() {
  return new StereoProjector();
}

QPointF StereoProjector::scattered2det(const Vec3D &v) const {
  return normal2det(scattered2normal(v));
}

QPointF StereoProjector::scattered2det(const Vec3D& v, bool& b) const {
  Vec3D t(scattered2normal(v,b));
  if (b) {
    return normal2det(t,b);
  } else {
    return QPointF();
  }
}

Vec3D StereoProjector::det2scattered(const QPointF& p) const {
  return normal2scattered(det2normal(p));
}

Vec3D StereoProjector::det2scattered(const QPointF& p, bool& b) const {
  Vec3D t(det2normal(p,b));
  if (b) {
    return normal2scattered(t,b);
  } else {
    return Vec3D();
  }
}

QPointF StereoProjector::normal2det(const Vec3D& n) const {
  Vec3D v=localCoordinates*n;
  double s=1.0+v.x();
  if (s<1e-5) {
    return QPointF();
  }
  return QPointF(v.y()/s, v.z()/s);
}

QPointF StereoProjector::normal2det(const Vec3D& n, bool& b) const {
  Vec3D v=localCoordinates*n;
  double s=1.0+v.x();
  if (s<1e-5) {
    b=false;
    return QPointF();
  }
  b=true;
  return QPointF(v.y()/s, v.z()/s);
}

Vec3D StereoProjector::det2normal(const QPointF& p) const {
  double x=p.x();
  double y=p.y();
  double n=1.0/(x*x+y*y+1.0);
  return localCoordinates.transposed()*Vec3D(n*(1.0-x*x-y*y), 2*x*n, 2*y*n);
}

Vec3D StereoProjector::det2normal(const QPointF& p, bool& b) const {
  double x=p.x();
  double y=p.y();
  double n=1.0/(x*x+y*y+1.0);
  b=true;
  return localCoordinates.transposed()*Vec3D(n*(1.0-x*x-y*y), 2*x*n, 2*y*n);
}

bool StereoProjector::project(const Reflection &r, QPointF &p) {
  bool doesReflect=false;
  for (int i=0; i<r.orders.size(); i++) {
    int n=r.orders[i];
    if ((2.0*QminVal<=n*r.Q) and (n*r.Q<=2.0*QmaxVal)) {
      doesReflect=true;
      break;
    }
  }
  if (not doesReflect)
    return false;

  Vec3D v=localCoordinates*r.normal;
  double s=1.0+v.x();
  if (s>1e-5) {
    s=1.0/s;
    p.setX(v.y()*s);
    p.setY(v.z()*s);
    return true;
  }
  return false;
}


void StereoProjector::decorateScene() {
  while (!decorationItems.empty()) {
    QGraphicsItem* item = decorationItems.takeLast();
    scene.removeItem(item);
    delete item;
  }
  decorationItems.append(scene.addEllipse(-1.0, -1.0, 2.0, 2.0, QPen(Qt::gray)));
  decorationItems.append(scene.addLine(-1.0, 0.0, 1.0, 0.0, QPen(Qt::gray)));
  decorationItems.append(scene.addLine(0.0, -1.0, 0.0, 1.0, QPen(Qt::gray)));

  QList<QPointF> items;
  items << QPointF(1,0) << QPointF(-1,0) << QPointF(0,1) << QPointF(0,-1);
  foreach (QPointF p, items) {

    Vec3D c = det2normal(p);
    QString s;
    if (c==Vec3D(1,0,0)) {
      s="x";
    } else if (c==Vec3D(-1,0,0)) {
      s="<span style=""text-decoration:overline"">x</span>";
    } else if (c==Vec3D(0,1,0)) {
      s="y";
    } else if (c==Vec3D(0,-1,0)) {
      s="<span style=""text-decoration:overline"">y</span>";
    } else if (c==Vec3D(0,0,1)) {
      s="z";
    } else if (c==Vec3D(0,0,-1)) {
      s="<span style=""text-decoration:overline"">z</span>";
    } else {
     s = QString("%1 %2 %3").arg(c.x()).arg(c.y()).arg(c.z());
    }
    QGraphicsTextItem* ti = new QGraphicsTextItem();
    ti->setTransform(QTransform(1,0,0,-1,0,0));
    ti->setHtml(s);

    ti->setPos(p);
    QRectF r=ti->boundingRect();
    double sc=getTextSize()/std::min(r.width(), r.height());
    ti->scale(sc,sc);

    ti->moveBy( std::min(1.0-ti->sceneBoundingRect().right(), 0.0), 0);
    ti->moveBy(-std::min(1.0+ti->sceneBoundingRect().left() , 0.0), 0);
    ti->moveBy(0,  std::min(1.0-ti->sceneBoundingRect().bottom(), 0.0));
    ti->moveBy(0, -std::min(1.0+ti->sceneBoundingRect().top(), 0.0));

    scene.addItem(ti);
    decorationItems << ti;
  }

}

QWidget* StereoProjector::configWidget() {
  return new StereoCfg(this);
}

QString StereoProjector::projectorName() {
  return QString("StereoProjector");
}

QString StereoProjector::displayName() {
  return QString("Stereographic Projection");
}

void StereoProjector::setDetOrientation(const Mat3D& M) {
  localCoordinates=M;
  decorateScene();
  if (projectionEnabled)
    emit projectionParamsChanged();
}

Mat3D StereoProjector::getDetOrientation() {
  return localCoordinates;
}

void StereoProjector::saveToXML(QDomElement base) {
  QDomDocument doc = base.ownerDocument();
  QDomElement projector = (base.tagName()=="Projector") ? base : base.appendChild(doc.createElement("Projector")).toElement();
  QDomElement e = projector.appendChild(doc.createElement("Frame")).toElement();
  e.setAttribute("M00", localCoordinates(0,0));
  e.setAttribute("M01", localCoordinates(0,1));
  e.setAttribute("M02", localCoordinates(0,2));
  e.setAttribute("M10", localCoordinates(1,0));
  e.setAttribute("M11", localCoordinates(1,1));
  e.setAttribute("M12", localCoordinates(1,2));
  e.setAttribute("M20", localCoordinates(2,0));
  e.setAttribute("M21", localCoordinates(2,1));
  e.setAttribute("M22", localCoordinates(2,2));

  Projector::saveToXML(projector);
}

bool StereoProjector::parseXMLElement(QDomElement e) {
  bool ok;
  if (e.tagName()=="Frame") {
    Mat3D M;
    M(0,0) = e.attribute("M00").toDouble(&ok); if (!ok) return false;
    M(0,1) = e.attribute("M01").toDouble(&ok); if (!ok) return false;
    M(0,2) = e.attribute("M02").toDouble(&ok); if (!ok) return false;
    M(1,0) = e.attribute("M10").toDouble(&ok); if (!ok) return false;
    M(1,1) = e.attribute("M11").toDouble(&ok); if (!ok) return false;
    M(1,2) = e.attribute("M12").toDouble(&ok); if (!ok) return false;
    M(2,0) = e.attribute("M20").toDouble(&ok); if (!ok) return false;
    M(2,1) = e.attribute("M21").toDouble(&ok); if (!ok) return false;
    M(2,2) = e.attribute("M22").toDouble(&ok); if (!ok) return false;
    setDetOrientation(M);
  } else {
    return Projector::parseXMLElement(e);
  }
  return true;
}

bool StereoProjector_registered = ProjectorFactory::registerProjector("StereoProjector", &StereoProjector::getInstance);
