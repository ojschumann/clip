/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

#include "core/stereoprojector.h"

#include <cmath>
#include <QtGui/QGraphicsEllipseItem>
#include <iostream>
#include <QSettings>

#include "ui/stereocfg.h"
#include "tools/vec3D.h"
#include "core/reflection.h"
#include "core/projectorfactory.h"
#include "tools/xmltools.h"

using namespace std;

StereoProjector::StereoProjector(QObject* parent):
    Projector(parent),
    localCoordinates() {
  internalSetWavevectors(0, M_PI);

  QSettings settings;
  settings.beginGroup(projectorName());
  internalSetWavevectors(settings.value("Qmin", 0.0).toDouble(), settings.value("Qmax", M_PI).toDouble());
  setMaxHklSqSum(settings.value("maxHKLSqSum", 3).toInt());
  setTextSizeFraction(settings.value("textSizeFraction", 10.0).toDouble());
  setSpotSizeFraction(settings.value("spotSizeFraction",  1.0).toDouble());
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      localCoordinates(i,j)=settings.value(QString("Frame%1%2").arg(i).arg(j), (i==j)?1.0:0.0).toDouble();
    }
  }

  settings.endGroup();

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

QString StereoProjector::projectorName() const {
  return QString("StereoProjector");
}

QString StereoProjector::displayName() {
  return QString("Stereographic Projection");
}

QSize StereoProjector::projectorSizeHint() const {
  QSettings settings;
  return settings.value(QString("%1/windowSize").arg(projectorName()), QSize(200, 250)).toSize();
}

void StereoProjector::setDetOrientation(const Mat3D& M) {
  localCoordinates=M;
  decorateScene();
  emit projectionParamsChanged();
}

Mat3D StereoProjector::getDetOrientation() {
  return localCoordinates;
}


const char XML_Stereo_Frame[] = "Frame";
const char XML_Stereo_Frame_Mxx[] = "M%1%2";


QDomElement StereoProjector::saveToXML(QDomElement base) {
  QDomElement projector = Projector::saveToXML(base);
  QDomElement e = projector.appendChild(projector.ownerDocument().createElement(XML_Stereo_Frame)).toElement();
  for (int i=0; i<3;  i++) {
    for (int j=0; j<3;  j++) {
      e.setAttribute(QString(XML_Stereo_Frame_Mxx).arg(i).arg(j), localCoordinates(i,j));
    }
  }
  return projector;
}

bool StereoProjector::parseXMLElement(QDomElement e) {
  bool ok=true;
  if (e.tagName()==XML_Stereo_Frame) {
    Mat3D M;
    for (int i=0; i<3;  i++) {
      for (int j=0; j<3;  j++) {
        M(i,j)=readDouble(e, QString(XML_Stereo_Frame_Mxx).arg(i).arg(j), ok);
      }
    }
    if (ok) setDetOrientation(M);
  } else {
    return Projector::parseXMLElement(e);
  }
  return ok;
}

void StereoProjector::saveParametersAsDefault() {
  QSettings settings;
  settings.beginGroup(projectorName());
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      settings.setValue(QString("Frame%1%2").arg(i).arg(j), localCoordinates(i,j));
    }
  }
  settings.endGroup();
  Projector::saveParametersAsDefault();
}

bool StereoProjector_registered = ProjectorFactory::registerProjector("StereoProjector", &StereoProjector::getInstance);
