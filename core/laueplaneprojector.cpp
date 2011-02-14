#include "core/laueplaneprojector.h"

#include <QCursor>
#include <QPointF>
#include <QSettings>
#include <cmath>
#include <iostream>
#include <typeinfo>

#include "defs.h"
#include "ui/laueplanecfg.h"
#include "core/reflection.h"
#include "core/projectorfactory.h"
#include "image/laueimage.h"
#include "tools/circleitem.h"
#include "tools/xmltools.h"
#include "config/configstore.h"

using namespace std;

LauePlaneProjector::LauePlaneProjector(QObject* parent):
    Projector(parent),
    localCoordinates(),
    distGroup(this),
    orientationGroup(this),
    shiftGroup(this)
{
  QSettings settings;
  settings.beginGroup(projectorName());
  internalSetWavevectors(settings.value("Qmin", 0.0).toInt(), settings.value("Qmax", 2.0*M_PI).toInt());
  setDetSize(settings.value("detDist", 30.0).toInt(), settings.value("detWidth", 110.0).toInt(), settings.value("detHeight", 140.0).toInt());
  setDetOrientation(settings.value("detOmega", 180.0).toInt(), settings.value("detChi", 0).toInt(), settings.value("detPhi", 0).toInt());
  detDx=1.0;
  detDy=1.0;
  setDetOffset(settings.value("detDX", 0.0).toInt(), settings.value("detDY", 0.0).toInt());
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
    cerr << "Copy operator on LauePlaneProjector " << endl;
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
  return QPointF(w.y()/w.x()+detDx, w.z()/w.x()+detDy);
}

QPointF LauePlaneProjector::scattered2det(const Vec3D &v, bool& b) const{
  Vec3D w=localCoordinates*v;
  if (w.x()<=0.0) {
    b=false;
    return QPointF();
  }
  b=true;
  return QPointF(w.y()/w.x()+detDx, w.z()/w.x()+detDy);
}

Vec3D LauePlaneProjector::det2scattered(const QPointF& p) const{
  Vec3D v(1.0 , p.x()-detDx, p.y()-detDy);
  v.normalize();
  return localCoordinates.transposed()*v;
}

Vec3D LauePlaneProjector::det2scattered(const QPointF& p, bool& b) const{
  Vec3D v(1.0 , p.x()-detDx, p.y()-detDy);
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

Vec3D LauePlaneProjector::det2normal(const QPointF& p, bool& b)  const {
  Vec3D v(det2scattered(p, b));
  if (b) {
    return scattered2normal(v,b);
  } else {
    return Vec3D();
  }
}

void LauePlaneProjector::setDetSize(double dist, double width, double height) {
  if ((detDist!=dist) or (detWidth!=width) or (detHeight!=height)) {
    detDist=dist;
    detWidth=width;
    detHeight=height;

    scene.setSceneRect(QRectF(-0.5*detWidth/detDist, -0.5*detHeight/detDist, detWidth/detDist, detHeight/detDist));

    emit projectionRectSizeChanged();
    emit projectionParamsChanged();
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
  updatePBPos();
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


bool LauePlaneProjector::project(const Reflection &r, QPointF& p) {
  if (r.lowestDiffOrder==0)
    return false;

  bool doesReflect=false;
  for (int i=0; i<r.orders.size(); i++) {
    int n=r.orders[i];
    if ((2.0*QminVal<=n*r.Qscatter) and (n*r.Qscatter<=2.0*QmaxVal)) {
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
  p.setX(v.y()*s+detDx);
  p.setY(v.z()*s+detDy);
  return true;
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

  CircleItem* marker=new CircleItem(0.1, center);
  ConfigStore::getInstance()->ensureColor(ConfigStore::PrimaryBeamMarker, marker, SLOT(setColor(QColor)));

  CircleItem* handle=new CircleItem(getSpotSize(), center);
  ConfigStore::getInstance()->ensureColor(ConfigStore::PrimaryBeamMarker, handle, SLOT(setColor(QColor)));
  handle->setPos(0.2, 0);
  handle->setFlag(QGraphicsItem::ItemIsMovable, true);
  handle->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  handle->setCursor(Qt::SizeAllCursor);

  connect(this, SIGNAL(spotSizeChanged(double)), center, SLOT(setRadius(double)));
  connect(this, SIGNAL(spotSizeChanged(double)), handle, SLOT(setRadius(double)));

  decorationItems.append(center);
  decorationItems.append(handle);
  decorationItems.append(marker);

  updatePBPos();


  connect(center, SIGNAL(positionChanged()), this, SLOT(movedPBMarker()));
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

void LauePlaneProjector::movedPBMarker() {
  if ((decorationItems.size()<3) && !isProjectionEnabled())
    return;

  CircleItem* center=dynamic_cast<CircleItem*>(decorationItems[0]);
  QPointF p=center->scenePos();

  bool b=false;
  QPointF q;
  if (omega()>90.5) {
    q=(scattered2det(Vec3D(1,0,0), b));
  } else if (omega()<89.5) {
    q=scattered2det(Vec3D(-1,0,0), b);
  }
  qDebug() << "movedPbMarker" << p << q;
  if (b) {
    setDetOffset(xOffset()+(p.x()-q.x())*dist(), yOffset()+(p.y()-q.y())*dist());
  }
}

void LauePlaneProjector::updatePBPos() {
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
      qDebug() << "updatePbPos" << center->scenePos() << q;
      center->setPosNoSig(det2img.map(q));
    }
  }
}

QWidget* LauePlaneProjector::configWidget() {
  return new LauePlaneCfg(this);
}

QString LauePlaneProjector::projectorName() {
  return QString("LauePlaneProjector");
}

QString LauePlaneProjector::displayName() {
  return QString("Laue Plane");
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

void LauePlaneProjector::doImgRotation(const QTransform& t) {
  if (getLaueImage() && getLaueImage()->hasAbsoluteSize()) {
    QSizeF imgSize = getLaueImage()->transformedAbsoluteSize();
    setDetSize(dist(), imgSize.width(), imgSize.height());
  } else {
    QTransform Tinv = t.inverted();
    QPointF c = Tinv.map(QPointF(0,0));
    QPointF x = Tinv.map(QPointF(1,0));
    double dw = fasthypot((x.x()-c.x())*detWidth, (x.y()-c.y())*detHeight);
    x = Tinv.map(QPointF(0,1));
    double dh = fasthypot((x.x()-c.x())*detWidth, (x.y()-c.y())*detHeight);
    setDetSize(dist(), dw, dh);
  }
  Projector::doImgRotation(t);
}

void LauePlaneProjector::loadParmetersFromImage(LaueImage *img) {
  QSizeF imgSize;
  if (img->hasAbsoluteSize()) {
    imgSize = img->transformedAbsoluteSize();
  } else {
    imgSize = img->transformedSize();
    imgSize *= sqrt(width()*height()/(imgSize.width()*imgSize.height()));
  }
  setDetSize(dist(), imgSize.width(), imgSize.height());
}

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

double LauePlaneProjector::DistGroup::value(int member) const {
  return projector->dist();
}

void LauePlaneProjector::DistGroup::doSetValue(QList<double> values) {
  projector->setDetSize(values.at(0), projector->width(), projector->height());
}

double LauePlaneProjector::DistGroup::epsilon(int member) const {
  return 0.001;
}

double LauePlaneProjector::DistGroup::lowerBound(int member) const {
  return 5.0;
}

double LauePlaneProjector::DistGroup::upperBound(int member) const {
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

double LauePlaneProjector::ShiftGroup::epsilon(int member) const {
  return 0.0001;
}

double LauePlaneProjector::ShiftGroup::lowerBound(int member) const {
  return -20.0;
}

double LauePlaneProjector::ShiftGroup::upperBound(int member) const {
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

double LauePlaneProjector::OrientationGroup::epsilon(int member) const {
  return 0.0001;
}

double LauePlaneProjector::OrientationGroup::lowerBound(int member) const {
  return -20.0;
}

double LauePlaneProjector::OrientationGroup::upperBound(int member) const {
  return 380;
}

void LauePlaneProjector::saveParametersAsDefault() {
  QSettings settings;
  settings.beginGroup(projectorName());
  settings.setValue("Qmin", Qmin());
  settings.setValue("Qmax", Qmax());
  settings.setValue("detDist", dist());
  settings.setValue("detWidth", width());
  settings.setValue("detHeight", height());
  settings.setValue("detOmega", omega());
  settings.setValue("detChi", chi());
  settings.setValue("detPhi", phi());
  settings.setValue("detDX", xOffset());
  settings.setValue("detDY", yOffset());
  settings.endGroup();
}

bool LauePlaneProjector_registered = ProjectorFactory::registerProjector("LauePlaneProjector", &LauePlaneProjector::getInstance);
