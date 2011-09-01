/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

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

#include "core/laueplaneprojector.h"

#include <QCursor>
#include <QPointF>
#include <QSettings>
#include <cmath>

#include <typeinfo>

#include "defs.h"
#include "ui/laueplanecfg.h"
#include "core/reflection.h"
#include "core/projectorfactory.h"
#include "core/crystal.h"
#include "image/laueimage.h"
#include "tools/tools.h"
#include "tools/circleitem.h"
#include "tools/xmltools.h"
#include "config/configstore.h"


using namespace std;

static const double sceneBlowup = 1000.0;

const char XML_LPP_DetSize[] = "DetSize";
const char XML_LPP_DetSize_dist[] = "dist";
const char XML_LPP_DetSize_width[] = "width";
const char XML_LPP_DetSize_height[] = "height";
const char XML_LPP_DetOrientation[] = "DetOrientation";
const char XML_LPP_DetOrientation_omega[] = "Omega";
const char XML_LPP_DetOrientation_chi[] = "Chi";
const char XML_LPP_DetOrientation_phi[] = "Phi";
const char XML_LPP_DetOffset[] = "DetOffset";
const char XML_LPP_DetOffset_x[] = "xOffset";
const char XML_LPP_DetOffset_y[] = "yOffset";

LauePlaneProjector::LauePlaneProjector(QObject* parent):
    Projector(parent),
    localCoordinates(),
    distGroup(this),
    orientationGroup(this),
    shiftGroup(this)
{
  QSettings settings;
  settings.beginGroup(projectorName());
  internalSetWavevectors(settings.value(Settings_QRangeMin, 0.0).toDouble(), settings.value(Settings_QRangeMax, 2.0*M_PI).toDouble());
  setMaxHklSqSum(settings.value(Settings_maxHKLSqSum, 3).toInt());
  setTextSizeFraction(settings.value(Settings_textSizeFraction, 10.0).toDouble());
  setSpotSizeFraction(settings.value(Settings_spotSizeFraction,  1.0).toDouble());

  setDetSize(settings.value("detDist", 30.0).toDouble(), settings.value("detWidth", 150.0).toDouble(), settings.value("detHeight", 150.0).toDouble());
  setDetOrientation(settings.value("detOmega", 180.0).toDouble(), settings.value("detChi", 0).toDouble(), settings.value("detPhi", 0).toDouble());
  detDx=1.0;
  detDy=1.0;
  setDetOffset(settings.value("detDX", 0.0).toDouble(), settings.value("detDY", 0.0).toDouble());
  settings.endGroup();

  addParameterGroup(&distGroup);
  addParameterGroup(&orientationGroup);
  addParameterGroup(&shiftGroup);

  connect(this, SIGNAL(projectionParamsChanged()), &distGroup, SLOT(groupDataChanged()));
  connect(this, SIGNAL(projectionParamsChanged()), &shiftGroup, SLOT(groupDataChanged()));
  connect(this, SIGNAL(projectionParamsChanged()), &orientationGroup, SLOT(groupDataChanged()));

  connect(this, SIGNAL(imageLoaded(LaueImage*)), this, SLOT(loadParmetersFromImage(LaueImage*)));

};

Projector& LauePlaneProjector::operator=(const Projector& _o) {
  LauePlaneProjector const* o = dynamic_cast<LauePlaneProjector const*>(&_o);
  if (o) {
    Projector::operator =(_o);
    setDetSize(o->dist(), o->width(), o->height());
    setDetOrientation(o->omega(), o->chi(), o->phi());
    setDetOffset(o->xOffset(), o->yOffset());
  } else {
    qDebug() << "Copy operator on LauePlaneProjector ";
  }
  return *this;
}

Projector* LauePlaneProjector::getInstance() {
  return new LauePlaneProjector();
}

QPointF LauePlaneProjector::scattered2det(const Vec3D &v) const{
  Vec3D w=localCoordinates*v;
  if (w.x()<=0.0) {
    return QPointF();
  }
  return QPointF(sceneBlowup*(w.y()/w.x()+detDx), sceneBlowup*(w.z()/w.x()+detDy));
}

QPointF LauePlaneProjector::scattered2det(const Vec3D &v, bool& b) const{
  Vec3D w=localCoordinates*v;
  if (w.x()<=0.0) {
    b=false;
    return QPointF();
  }
  b=true;
  return QPointF(sceneBlowup*(w.y()/w.x()+detDx), sceneBlowup*(w.z()/w.x()+detDy));
}

Vec3D LauePlaneProjector::det2scattered(const QPointF& p) const{
  Vec3D v(1.0 , p.x()/sceneBlowup-detDx, p.y()/sceneBlowup-detDy);
  v.normalize();
  return localCoordinates.transposed()*v;
}

Vec3D LauePlaneProjector::det2scattered(const QPointF& p, bool& b) const{
  Vec3D v(1.0 , p.x()/sceneBlowup-detDx, p.y()/sceneBlowup-detDy);
  v.normalize();
  b=true;
  return localCoordinates.transposed()*v;
}

QPointF LauePlaneProjector::normal2det(const Vec3D& n) const{
  return scattered2det(normal2scattered(n));
}

QPointF LauePlaneProjector::normal2det(const Vec3D& n, bool& b) const{
  Vec3D v(normal2scattered(n, b));
  if (b) {
    return scattered2det(v,b);
  } else {
    return QPointF();
  }
}

Vec3D LauePlaneProjector::det2normal(const QPointF& p)  const {
  return scattered2normal(det2scattered(p));
}

Vec3D LauePlaneProjector::det2normal(const QPointF &p, bool &b)  const {
  Vec3D v(det2scattered(p, b));
  if (b) {
    return scattered2normal(v,b);
  } else {
    return Vec3D();
  }
}

QPair<double, double> LauePlaneProjector::validOrderRange(double /*Q*/, double Qscatter) {
  if (Qscatter<1e-5) return qMakePair(0.0, 0.0);
  return qMakePair(2.0*QminVal/Qscatter, 2.0*QmaxVal/Qscatter);
}

bool LauePlaneProjector::project(const Reflection &r, QPointF& p) {
  if (r.lowestDiffOrder==0)
    return false;

  QPair<double, double> limits = validOrderRange(r.Q, r.Qscatter);
  bool doesReflect=false;
  for (int i=0; i<r.orders.size(); i++) {
    int n=r.orders[i];
    if ((limits.first<=n) and (n<=limits.second)) {
      doesReflect=true;
      break;
    }
  }
  if (not doesReflect)
    return false;

  Vec3D v=localCoordinates*r.scatteredRay;
  double s=v.x();
  if (s<1e-10)
    return false;


  s=1.0/s;
  p.setX((v.y()*s+detDx)*sceneBlowup);
  p.setY((v.z()*s+detDy)*sceneBlowup);
  return true;
}


void LauePlaneProjector::setDetSize(double dist, double width, double height) {
  if ((detDist!=dist) or (detWidth!=width) or (detHeight!=height)) {
    detDist=dist;
    detWidth=width;
    detHeight=height;

    scene.setSceneRect(QRectF(-0.5*sceneBlowup*detWidth/detDist, -0.5*sceneBlowup*detHeight/detDist, sceneBlowup*detWidth/detDist, sceneBlowup*detHeight/detDist));

    emit projectionRectSizeChanged();
    emit projectionParamsChanged();

    if (getLaueImage())
      getLaueImage()->data()->setTransformedSizeData(ImageDataStore::PhysicalSize, QSizeF(detWidth, detHeight));
  }
}

void LauePlaneProjector::setDetOrientation(double omega, double chi, double phi) {
  if ((detOmega!=omega) or (detChi!=chi) or (detPhi!=phi)) {
    
    // Save detector offsets
    Vec3D w1 = localCoordinates * Vec3D(1, 0, 0);

    detOmega=omega;
    detChi=chi;
    detPhi=phi;
    localCoordinates =Mat3D(Vec3D(0,0,1), M_PI*(omega-180.0)/180.0);
    localCoordinates*=Mat3D(Vec3D(0,1,0), M_PI*chi/180.0);
    localCoordinates*=Mat3D(Vec3D(1,0,0), M_PI*phi/180.0);

    //
    Vec3D w2 = localCoordinates * Vec3D(1, 0, 0);

    //w1.y()/w1.x() + detx_old == w2.y()/w2.x() + detx_new
    detDx += w1.y()/w1.x() - w2.y()/w2.x();
    detDy += w1.z()/w1.x() - w2.z()/w2.x();

    emit projectionParamsChanged();
  }
}

void LauePlaneProjector::setDetOffset(double dx, double dy) {
  dx/=dist();
  dy/=dist();
  detDx=dx;
  detDy=dy;
  updatePrimaryBeamPos();
  emit projectionParamsChanged();
}


void LauePlaneProjector::setDist(double v) {
  setDetSize(v, width(), height());
}

void LauePlaneProjector::setWidth(double v) {
  setDetSize(dist(), v, height());
}

void LauePlaneProjector::setHeight(double v) {
  setDetSize(dist(), width(), v);
}

void LauePlaneProjector::setOmega(double v) {
  setDetOrientation(v, chi(), phi());
}

void LauePlaneProjector::setChi(double v) {
  setDetOrientation(omega(), v, phi());
}

void LauePlaneProjector::setPhi(double v) {
  setDetOrientation(omega(), chi(), v);
}

void LauePlaneProjector::setXOffset(double v) {
  setDetOffset(v, yOffset());
}

void LauePlaneProjector::setYOffset(double v) {
  setDetOffset(xOffset(), v);
}


void LauePlaneProjector::decorateScene() {
  while (!decorationItems.empty()) {
    QGraphicsItem* item = decorationItems.takeLast();
    scene.removeItem(item);
    delete item;
  }

  CircleItem* center=new CircleItem(getSpotSize(), imageItemsPlane);


  ConfigStore::getInstance()->ensureColor(ConfigStore::PrimaryBeamMarker, center, SLOT(setColor(QColor)));
  center->setFlag(QGraphicsItem::ItemIsMovable, true);
  center->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  center->setCursor(Qt::SizeAllCursor);
  center->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  center->setLineWidth(1.0);

  CircleItem* marker=new CircleItem(0.1, center);
  ConfigStore::getInstance()->ensureColor(ConfigStore::PrimaryBeamMarker, marker, SLOT(setColor(QColor)));
  marker->setLineWidth(1.0);

  CircleItem* handle=new CircleItem(getSpotSize(), center);
  ConfigStore::getInstance()->ensureColor(ConfigStore::PrimaryBeamMarker, handle, SLOT(setColor(QColor)));
  handle->setPos(0.05*scene.width(), 0);
  handle->setFlag(QGraphicsItem::ItemIsMovable, true);
  handle->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  handle->setCursor(Qt::SizeAllCursor);
  handle->setLineWidth(1.0);

  connect(this, SIGNAL(spotSizeChanged(double)), center, SLOT(setRadius(double)));
  connect(this, SIGNAL(spotSizeChanged(double)), handle, SLOT(setRadius(double)));

  decorationItems.append(center);
  decorationItems.append(handle);
  decorationItems.append(marker);

  updatePrimaryBeamPos();


  connect(center, SIGNAL(positionChanged()), this, SLOT(movedPrimaryBeamMarker()));
  connect(handle, SIGNAL(positionChanged()), this, SLOT(resizePBMarker()));
  resizePBMarker();
}


void LauePlaneProjector::resizePBMarker() {
  if ((decorationItems.size()<3) && !isProjectionEnabled())
    return;

  //CircleItem* center=dynamic_cast<CircleItem*>(decorationItems[0]);
  CircleItem* handle=dynamic_cast<CircleItem*>(decorationItems[1]);
  CircleItem* marker=dynamic_cast<CircleItem*>(decorationItems[2]);

  QPointF p=handle->pos();
  double l=fasthypot(p.x(), p.y());

  marker->setRadius(l);
}

void LauePlaneProjector::movedPrimaryBeamMarker() {
  if ((decorationItems.size()<3) && !isProjectionEnabled())
    return;

  CircleItem* center=dynamic_cast<CircleItem*>(decorationItems[0]);
  QPointF p=img2det.map(center->pos());

  bool b=false;
  QPointF q;
  if (omega()>90.5) {
    q=(scattered2det(Vec3D(1,0,0), b));
  } else if (omega()<89.5) {
    q=scattered2det(Vec3D(-1,0,0), b);
  }
  if (b) {
    setDetOffset(xOffset()+(p.x()-q.x())*dist()/sceneBlowup, yOffset()+(p.y()-q.y())*dist()/sceneBlowup);
  }
}

void LauePlaneProjector::updatePrimaryBeamPos() {
  if ((decorationItems.size()>2) && isProjectionEnabled()) {
    bool b=false;
    QPointF q;
    if (omega()>90.5) {
      q=(scattered2det(Vec3D(1,0,0), b));
    } else if (omega()<89.5) {
      q=scattered2det(Vec3D(-1,0,0), b);
    }
    if (b) {
      CircleItem* center=dynamic_cast<CircleItem*>(decorationItems[0]);
      q = det2img.map((q));
      center->setPosNoSig(q);
    }
  }
}

QWidget* LauePlaneProjector::configWidget() {
  return new LauePlaneCfg(this);
}

QString LauePlaneProjector::projectorName() const {
  return QString("LauePlaneProjector");
}

QSize LauePlaneProjector::projectorSizeHint() const {
  QSettings settings;
  return settings.value(QString("%1/windowSize").arg(projectorName()), QSize(340, 375)).toSize();
}

QString LauePlaneProjector::displayName() {
  return QString("Laue Plane");
}

QString LauePlaneProjector::fillInfoTable(const QString &_html) {
  QString html(_html);
  html.replace("<DIST/>", QString::number(dist(), 'f', 3));
  html.replace("<WIDTH/>", QString::number(width(), 'f', 3));
  html.replace("<HEIGHT/>", QString::number(height(), 'f', 3));
  html.replace("<OMEGA/>", QString::number(omega(), 'f', 3));
  html.replace("<CHI/>", QString::number(chi(), 'f', 3));
  html.replace("<PHI/>", QString::number(phi(), 'f', 3));
  html.replace("<DX/>", QString::number(xOffset(), 'f', 3));
  html.replace("<DY/>", QString::number(yOffset(), 'f', 3));
  html.replace("<QMIN/>", QString::number(Qmin(), 'f', 3));
  html.replace("<QMAX/>", QString::number(Qmax(), 'f', 3));
  return html;
}


double LauePlaneProjector::dist() const {
  return detDist;
}

double LauePlaneProjector::width() const {
  return detWidth;
}

double LauePlaneProjector::height() const {
  return detHeight;
}
double LauePlaneProjector::omega() const {
  return detOmega;
}

double LauePlaneProjector::chi() const {
  return detChi;
}

double LauePlaneProjector::phi() const {
  return detPhi;
}

double LauePlaneProjector::xOffset() const {
  return detDx*dist();
}

double LauePlaneProjector::yOffset() const {
  return detDy*dist();
}


void LauePlaneProjector::loadParmetersFromImage(LaueImage *img) {
  connect(img->data(), SIGNAL(dataChanged(ImageDataStore::DataType,QVariant)), this, SLOT(loadNewPhysicalImageSize(ImageDataStore::DataType)));
  connect(img->data(), SIGNAL(transformChanged()), this, SLOT(loadNewPhysicalImageSize()));

  double d = dist();
  double w = width();
  double h = height();

  if (img->data()->hasData(ImageDataStore::PlaneDetectorToSampleDistance))
    d = img->data()->getData(ImageDataStore::PlaneDetectorToSampleDistance).toDouble();

  if (img->data()->hasData(ImageDataStore::PhysicalSize)) {
    QSizeF s = img->data()->getData(ImageDataStore::PhysicalSize).toSizeF();
    w = s.width();
    h = s.height();
  } else {
    QSizeF s = img->data()->getData(ImageDataStore::PixelSize).toSizeF();
    double scale = sqrt(w*h/(s.width()*s.height()));
    w = scale * s.width();
    h = scale * s.height();
    img->data()->setTransformedSizeData(ImageDataStore::PhysicalSize, QSizeF(w, h));
  }

  setDetSize(d, w, h);
}

QDomElement LauePlaneProjector::saveToXML(QDomElement base) {
  QDomDocument doc = base.ownerDocument();
  QDomElement projector = Projector::saveToXML(base);

  QDomElement e = projector.appendChild(doc.createElement(XML_LPP_DetSize)).toElement();
  e.setAttribute(XML_LPP_DetSize_dist, dist());
  e.setAttribute(XML_LPP_DetSize_width, width());
  e.setAttribute(XML_LPP_DetSize_height, height());

  e = projector.appendChild(doc.createElement(XML_LPP_DetOrientation)).toElement();
  e.setAttribute(XML_LPP_DetOrientation_omega, omega());
  e.setAttribute(XML_LPP_DetOrientation_chi, chi());
  e.setAttribute(XML_LPP_DetOrientation_phi, phi());

  e = projector.appendChild(doc.createElement(XML_LPP_DetOffset)).toElement();
  e.setAttribute(XML_LPP_DetOffset_x, xOffset());
  e.setAttribute(XML_LPP_DetOffset_y, yOffset());
  return projector;
}

bool LauePlaneProjector::parseXMLElement(QDomElement e) {
  bool ok=true;
  if (e.tagName()==XML_LPP_DetSize) {
    double detD = readDouble(e, XML_LPP_DetSize_dist, ok);
    double detW = readDouble(e, XML_LPP_DetSize_width, ok);
    double detH = readDouble(e, XML_LPP_DetSize_height, ok);
    if (ok) setDetSize(detD, detW, detH);
  } else if (e.tagName()==XML_LPP_DetOrientation) {
    double detC = readDouble(e, XML_LPP_DetOrientation_chi, ok);
    double detP = readDouble(e, XML_LPP_DetOrientation_phi, ok);
    double detO = readDouble(e, XML_LPP_DetOrientation_omega, ok);
    if (ok) setDetOrientation(detO, detC, detP);
  } else if (e.tagName()==XML_LPP_DetOffset) {
    double detX = readDouble(e, XML_LPP_DetOffset_x, ok);
    double detY = readDouble(e, XML_LPP_DetOffset_y, ok);
    if (ok) setDetOffset(detX, detY);
  } else {
    return Projector::parseXMLElement(e);
  }
  return ok;
}

double LauePlaneProjector::TTmax() const {
  Vec3D n(1.0, 0.0, 0.0);
  double mc=maxCos(n);
  return 180.0-180.0*acos(mc)/M_PI;
}

double LauePlaneProjector::TTmin() const {
  Vec3D n(-1.0, 0.0, 0.0);
  double mc=maxCos(n);
  return 180.0*acos(mc)/M_PI;
}


double LauePlaneProjector::maxCos(Vec3D n) const {
  double dx = 0.5*width()/dist();
  double dy = 0.5*height()/dist();

  bool b;
  QPointF p=scattered2det(n, b);
  if (b and (fabs(p.x())<dx) and (fabs(p.y())<dy))
    return 1.0;

  // The four corners of the plane, vectors are normalized!
  QList<Vec3D> corners;
  corners.append(det2scattered(QPointF( dx,  dy)));
  corners.append(det2scattered(QPointF(-dx,  dy)));
  corners.append(det2scattered(QPointF(-dx, -dy)));
  corners.append(det2scattered(QPointF( dx, -dy)));

  // small value
  double maxCosTT=-2.0;

  for (unsigned int i=0; i<4; i++) {
    // Check one corner
    Vec3D a(corners[i]);
    double cosTT=n*a;
    if (cosTT>maxCosTT)
      maxCosTT=cosTT;


    Vec3D b=corners[(i+1)%4]-corners[i];
    double denom = ((n*a)*(b*b)-(n*b)*(a*b));
    if (denom!=0.0) {
      double lmin = ((n*b)*(a*a)-(n*a)*(a*b))/denom;
      if ((lmin>=0.0) and (lmin<=1.0)) {
        Vec3D v=a+b*lmin;
        v.normalize();
        cosTT=n*v;
        if (cosTT>maxCosTT)
          maxCosTT=cosTT;
      }
    }
  }

  return maxCosTT;
}


LauePlaneProjector::DistGroup::DistGroup(LauePlaneProjector* p):
    FitParameterGroup(p),
    projector(p)
{
  addParameter("Distance");
}

double LauePlaneProjector::DistGroup::value(int /*member*/) const {
  return projector->dist();
}

void LauePlaneProjector::DistGroup::doSetValue(QList<double> values) {
  projector->setDetSize(values.at(0), projector->width(), projector->height());
}

double LauePlaneProjector::DistGroup::epsilon(int /*member*/) const {
  return 0.001;
}

double LauePlaneProjector::DistGroup::lowerBound(int /*member*/) const {
  return 5.0;
}

double LauePlaneProjector::DistGroup::upperBound(int /*member*/) const {
  return 300.0;
}


LauePlaneProjector::ShiftGroup::ShiftGroup(LauePlaneProjector* p):
    FitParameterGroup(p),
    projector(p)
{
  addParameter("Det_x");
  addParameter("Det_y");
}

double LauePlaneProjector::ShiftGroup::value(int member) const {
  if (member==0) {
    return projector->xOffset();
  } else if (member==1) {
    return projector->yOffset();
  }
  return -1.0;
}

void LauePlaneProjector::ShiftGroup::doSetValue(QList<double> values) {
  projector->setDetOffset(values.at(0), values.at(1));
}

double LauePlaneProjector::ShiftGroup::epsilon(int /*member*/) const {
  return 0.0001;
}

double LauePlaneProjector::ShiftGroup::lowerBound(int /*member*/) const {
  return -20.0;
}

double LauePlaneProjector::ShiftGroup::upperBound(int /*member*/) const {
  return 380;
}



LauePlaneProjector::OrientationGroup::OrientationGroup(LauePlaneProjector* p):
    FitParameterGroup(p),
    projector(p)
{
  addParameter("Omega");
  addParameter("Chi");
}

double LauePlaneProjector::OrientationGroup::value(int member) const {
  if (member==0) {
    return projector->omega();
  } else if (member==1) {
    return projector->chi();
  }
  return -1.0;
}

void LauePlaneProjector::OrientationGroup::doSetValue(QList<double> values) {
  projector->setDetOrientation(values.at(0), values.at(1), projector->phi());
}

double LauePlaneProjector::OrientationGroup::epsilon(int /*member*/) const {
  return 0.0001;
}

double LauePlaneProjector::OrientationGroup::lowerBound(int /*member*/) const {
  return -20.0;
}

double LauePlaneProjector::OrientationGroup::upperBound(int /*member*/) const {
  return 380;
}

void LauePlaneProjector::saveParametersAsDefault() {
  QSettings settings;
  settings.beginGroup(projectorName());
  settings.setValue("detDist", dist());
  settings.setValue("detWidth", width());
  settings.setValue("detHeight", height());
  settings.setValue("detOmega", omega());
  settings.setValue("detChi", chi());
  settings.setValue("detPhi", phi());
  settings.setValue("detDX", xOffset());
  settings.setValue("detDY", yOffset());
  settings.endGroup();
  Projector::saveParametersAsDefault();
}

void LauePlaneProjector::loadNewPhysicalImageSize(ImageDataStore::DataType t) {
  if ((t==ImageDataStore::PhysicalSize) && getLaueImage() && getLaueImage()->data()->hasData(ImageDataStore::PhysicalSize)) {
    QSizeF s = getLaueImage()->data()->getTransformedSizeData(t);
    if ((fabs(s.width()-width())>1e-3) || (fabs(s.height()-height())>1e-3))
      setDetSize(dist(), s.width(), s.height());
  }
}

bool LauePlaneProjector_registered = ProjectorFactory::registerProjector("LauePlaneProjector", &LauePlaneProjector::getInstance);
