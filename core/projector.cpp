#include "core/projector.h"

#include <cmath>
#include <iostream>
#include <QTimer>
#include <QCursor>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QWidget>
#include <QThreadPool>
#include <QGraphicsView>
#include <QMetaObject>

#include "tools/circleitem.h"
#include "tools/ruleritem.h"
#include "tools/zoneitem.h"
#include "tools/cropmarker.h"
#include "core/reflection.h"
#include "core/crystal.h"
#include "image/laueimage.h"
#include "tools/spotindicatorgraphicsitem.h"

using namespace std;

Projector::Projector(QObject *parent):
    QObject(parent),
    FitObject(),
    decorationItems(),
    textMarkerItems(),
    spotMarkerStore(),
    zoneMarkerStore(),
    rulerStore(),
    crystal(),
    scene(this),
    imageItemsPlane(new QGraphicsPixmapItem()),
    imageData(0),
    spotIndicator(new SpotIndicatorGraphicsItem())
{
  imageItemsPlane->setFlag(QGraphicsItem::ItemIsMovable, false);

  scene.addItem(spotIndicator);
  scene.addItem(imageItemsPlane);

  spotIndicator->stackBefore(imageItemsPlane);

  enableSpots();
  enableProjection();
  updateImgTransformations();

  internalSetWavevectors(0.0, 1.0*M_1_PI);
  setMaxHklSqSum(3);
  setTextSizeFraction(10.0);
  setSpotSizeFraction(1.0);

  connect(this, SIGNAL(spotSizeChanged(double)), this, SLOT(reflectionsUpdated()));
  connect(this, SIGNAL(textSizeChanged(double)), this, SLOT(reflectionsUpdated()));

  QTimer::singleShot(0, this, SLOT(decorateScene()));
  connect(this, SIGNAL(projectionParamsChanged()), this, SLOT(reflectionsUpdated()));
  connect(&scene, SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(updateImgTransformations()));
};

Projector::Projector(const Projector &p): QObject() { }

Projector::~Projector() {
}

void Projector::connectToCrystal(Crystal *c) {
  if (not crystal.isNull()) {
    disconnect(this, 0, crystal, 0);
    disconnect(crystal, 0, this, 0);
    crystal->removeProjector(this);
  }
  crystal=c;
  crystal->addProjector(this);
  connect(crystal, SIGNAL(reflectionsUpdate()), this, SLOT(reflectionsUpdated()));
  emit projectionParamsChanged();
}

double Projector::Qmin() const {
  return QminVal;
}

double Projector::Qmax() const {
  return QmaxVal;
}

double Projector::TTmax() const {
  return 180.0;
}

double Projector::TTmin() const {
  return 0.0;
}

void Projector::setWavevectors(double Qmin, double Qmax)  {
  if ((Qmin<Qmax) and ((Qmin!=QminVal) or (Qmax!=QmaxVal))) {
    internalSetWavevectors(Qmin, Qmax);
  }
}

void Projector::internalSetWavevectors(double Qmin, double Qmax)  {
  QmaxVal=Qmax;
  QminVal=Qmin;
  emit projectionParamsChanged();
  emit wavevectorsUpdated();

}

void Projector::addInfoItem(const QString& text, const QPointF& p) {
  QGraphicsRectItem* bg=new QGraphicsRectItem();
  bg->setTransform(QTransform(1,0,0,-1,0,0));
  bg->setPen(QPen(Qt::black));
  bg->setBrush(QBrush(QColor(0xF0,0xF0,0xF0)));
  bg->setPos(p);
  bg->setFlag(QGraphicsItem::ItemIsMovable, true);
  bg->setCursor(QCursor(Qt::SizeAllCursor));
  bg->setZValue(1);
  QGraphicsTextItem*  t = new QGraphicsTextItem(bg);
  t->setHtml(text);
  QRectF r=t->boundingRect();
  double s=getTextSize()/std::min(r.width(), r.height());

  bg->setRect(t->boundingRect());
  bg->scale(s,s);

  scene.addItem(bg);
  infoStore.addItem(bg);
}

ItemStore& Projector::infoItems() {
  return infoStore;
}


void Projector::reflectionsUpdated() {
  if (crystal.isNull() or not projectionEnabled)
    return;

  foreach (QGraphicsItem* item, textMarkerItems) {
    scene.removeItem(item);
    delete item;
  }
  textMarkerItems.clear();

  spotIndicator->coordinates.clear();
  QVector<Reflection> refs = crystal->getReflectionList();
  reflectionIsProjected.resize(refs.size());

  for (int i=0; i<refs.size(); i++) {
    QPointF p;
    if ((reflectionIsProjected[i] = project(refs[i], p))) {
      spotIndicator->coordinates << p;
      if (refs.at(i).hklSqSum<=maxHklSqSum) {
        QGraphicsTextItem* t = new QGraphicsTextItem();
        t->setTransform(QTransform(1,0,0,-1,0,0));
        t->setHtml(refs.at(i).toHtml());
        t->setPos(p);
        QRectF r=t->boundingRect();
        double s=getTextSize()/std::min(r.width(), r.height());
        t->scale(s,s);
        textMarkerItems.append(t);
        scene.addItem(t);
      }
    }
  }
  spotIndicator->setSpotsize(getSpotSize());
  spotIndicator->paintUntil = spotIndicator->coordinates.size();
  spotIndicator->pointsUpdated();
  emit projectedPointsUpdated();
}

Vec3D Projector::normal2scattered(const Vec3D &v) {
  double x=v.x();
  if (x<=0.0) {
    return Vec3D();
  }
  double y=v.y();
  double z=v.z();
  return Vec3D(2*x*x-1.0, 2.0*x*y, 2.0*x*z);
}

Vec3D Projector::normal2scattered(const Vec3D &v, bool& b) {
  double x=v.x();
  if (x<=0.0) {
    b=false;
    return Vec3D();
  }
  double y=v.y();
  double z=v.z();
  b=true;
  return Vec3D(2*x*x-1.0, 2.0*x*y, 2.0*x*z);
}

Vec3D Projector::scattered2normal(const Vec3D& v) {
  double x=v.x();
  double y=v.y();
  double z=v.z();

  x=sqrt(0.5*(x+1.0));
  if (x==0.0) {
    return Vec3D();
  }
  return Vec3D(x, 0.5*y/x, 0.5*z/x);
}

Vec3D Projector::scattered2normal(const Vec3D& v, bool& b) {
  double x=v.x();
  double y=v.y();
  double z=v.z();

  x=sqrt(0.5*(x+1.0));
  if (x==0.0) {
    b=false;
    return Vec3D();
  }
  b=true;
  return Vec3D(x, 0.5*y/x, 0.5*z/x);
}

Reflection Projector::getClosestReflection(const Vec3D& normal) {
  if (crystal.isNull())
    return Reflection();
  QVector<Reflection> r = crystal->getReflectionList();
  if (r.size()!=reflectionIsProjected.size()) cout << "Somthing horrible is going on..." << endl;
  int minIdx=-1;
  double minDist=0;
  for (int n=0; n<r.size(); n++) {
    if (reflectionIsProjected.at(n)) {
      double dist=(r[n].normal-normal).norm_sq();
      if (dist<minDist or minIdx<0) {
        minDist=dist;
        minIdx=n;
      }
    }
  }
  if (minIdx<0) {
    return Reflection();
  } else {
    return r[minIdx];
  }
}



void Projector::addRotation(const Vec3D& axis, double angle) {
  if (not crystal.isNull())
    crystal->addRotation(axis, angle);
}

void Projector::addRotation(const Mat3D& M) {
  if (not crystal.isNull())
    crystal->addRotation(M);
}

void Projector::setRotation(const Mat3D& M) {
  if (not crystal.isNull())
    crystal->setRotation(M);
}

QGraphicsScene* Projector::getScene() {
  return &scene;
}

LaueImage* Projector::getLaueImage() {
  return imageData;
}

Crystal* Projector::getCrystal() {
  return crystal;
}

int Projector::getMaxHklSqSum() const {
  return maxHklSqSum;
}

double Projector::getSpotSize() const {
  return spotSizeFraction*std::min(scene.sceneRect().width(), scene.sceneRect().height());
}

double Projector::getSpotSizeFraction() const {
  return 100.0*spotSizeFraction;
}

double Projector::getTextSize() const {
  return textSizeFraction*std::min(scene.sceneRect().width(), scene.sceneRect().height());
}

double Projector::getTextSizeFraction() const {
  return 100.0*textSizeFraction;
}

bool Projector::spotsEnabled() const {
  return showSpots;
}

void Projector::setMaxHklSqSum(int m) {
  maxHklSqSum=m;
  emit projectionParamsChanged();
}

void Projector::setTextSizeFraction(double d) {
  textSizeFraction=0.01*d;
  emit textSizeChanged(getTextSize());
}

void Projector::setSpotSizeFraction(double d) {
  spotSizeFraction=0.01*d;
  cout << "new spotsize " << d <<endl;
  emit spotSizeChanged(getSpotSize());
}

void Projector::enableSpots(bool b) {
  showSpots=b;
  spotIndicator->setVisible(b);
  emit projectionParamsChanged();
}

// ----------------------- Handling of Spot Markers -------------
void Projector::addSpotMarker(const QPointF& p) {
  CircleItem* item = new CircleItem(getSpotSize(), imageItemsPlane);
  item->setFlag(QGraphicsItem::ItemIsMovable, true);
  item->setCursor(QCursor(Qt::SizeAllCursor));
  item->setColor(QColor(0xFF,0xAA,0x33));
  connect(this, SIGNAL(spotSizeChanged(double)), item, SLOT(setRadius(double)));
  item->setPos(det2img.map(p));
  item->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  spotMarkers().addItem(item);
}

ItemStore& Projector::spotMarkers() {
  return spotMarkerStore;
}

QPointF Projector::getSpotMarkerDetPos(int n) {
  return img2det.map(spotMarkers().at(n)->pos());
}

QList<Vec3D> Projector::getSpotMarkerNormals() {
  QList<Vec3D> r;
  for (int i=0; i<spotMarkers().size(); i++)
    r << det2normal(getSpotMarkerDetPos(i));
  return r;
}


// ---------------  Ruler handling ---------------------------
ItemStore& Projector::rulers() {
  return rulerStore;
}

void Projector::addRuler(const QPointF& p1, const QPointF& p2) {
  RulerItem* ruler = new RulerItem(det2img.map(p1), det2img.map(p2), getSpotSize(), imageItemsPlane);
  ruler->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  connect(this, SIGNAL(spotSizeChanged(double)), ruler, SLOT(setHandleSize(double)));
  rulers().addItem(ruler);
}

QPair<QPointF, QPointF> Projector::getRulerCoordinates(int n) {
  if (n<rulers().size()) {
    RulerItem* ruler = dynamic_cast<RulerItem*>(rulers().at(n));
    if (ruler)
      return qMakePair(img2det.map(ruler->getStart()), img2det.map(ruler->getEnd()));
  }
  return QPair<QPointF, QPointF>();
}



// ----------------------- Handling of Zone Markers -------------

void Projector::addZoneMarker(const QPointF& p1, const QPointF& p2) {
  ZoneItem* zoneMarker = new ZoneItem(det2img.map(p1), det2img.map(p2), this, imageItemsPlane);
  zoneMarker->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  connect(&spotMarkers(), SIGNAL(itemAdded(int)), zoneMarker, SLOT(updateOptimalZone()));
  connect(&spotMarkers(), SIGNAL(itemChanged(int)), zoneMarker, SLOT(updateOptimalZone()));
  connect(&spotMarkers(), SIGNAL(itemRemoved(int)), zoneMarker, SLOT(updateOptimalZone()));
  // Todo connect for update of spotsize
  zoneMarkers().addItem(zoneMarker);
}

QList<Vec3D> Projector::getZoneMarkerNormals() {
  QList<Vec3D> r;
  for (int i=0; i<zoneMarkers().size(); i++)
    r << dynamic_cast<ZoneItem*>(zoneMarkers().at(i))->getZoneNormal();
  return r;
}

ItemStore& Projector::zoneMarkers() {
  return zoneMarkerStore;
}

// ------------ Handling of Crop Marker ---------------
void Projector::showCropMarker() {
  if (cropMarker.isNull()) {
    //cropMarker = new CropMarker(QPointF(0.1, 0.1), 0.8, 0.8, 0.0, getSpotSize(), imageItemsPlane);
    cropMarker = new CropMarker(QPointF(0.1, 0.1), 0.8, 0.8, 0.0, getSpotSize());
    scene.addItem(cropMarker);

    connect(this, SIGNAL(spotSizeChanged(double)), cropMarker, SLOT(setHandleSize(double)));
    connect(cropMarker.data(), SIGNAL(cancelCrop()), this, SLOT(delCropMarker()), Qt::QueuedConnection);
    connect(cropMarker.data(), SIGNAL(publishCrop(QPolygonF)), this, SLOT(setCrop(QPolygonF)));
    //cropMarker->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  } else {
    cropMarker->show();
  }
}

void Projector::delCropMarker() {
  if (!cropMarker.isNull()) {
    scene.removeItem(cropMarker);
    delete cropMarker;
  }
}

void Projector::setCrop(QPolygonF rect) {
  QTransform t;
  if (QTransform::quadToSquare(det2img.map(rect), t)) {
    doImgRotation(t);
  }
}

CropMarker* Projector::getCropMarker() {
  return cropMarker;
}

void Projector::updateImgTransformations() {
  const QRectF r=scene.sceneRect();
  det2img.reset();
  if (r.isEmpty()) {
    img2det.reset();
  } else {
    det2img.scale(1.0/r.width(),  1.0/r.height());
    det2img.translate(-r.x(),  -r.y());
    img2det=det2img.inverted();
  }
  imageItemsPlane->setTransform(img2det);
  QTransform t = QTransform::fromScale(det2img.m11(), det2img.m22());
  foreach (QGraphicsItem* item, imageItemsPlane->childItems()) {
    item->setTransform(t);
  }
  emit imgTransformUpdated();
  emit spotSizeChanged(getSpotSize());
  emit textSizeChanged(getTextSize());
}

void Projector::loadImage(QString s) {
  LaueImage* tmpImage = new LaueImage(s, this);
  if (tmpImage->isValid()) {
    if (imageData) delete imageData;
    imageData = tmpImage;
    emit imageLoaded(imageData);
  } else {
    delete tmpImage;
  }
}

void Projector::closeImage() {
  if (imageData) delete imageData;
  imageData = 0;
  emit imageClosed();
}

// Rotates and flips the Decorations, which are bound to the Image
void Projector::doImgRotation(const QTransform& t) {
  foreach (QGraphicsItem* item, imageItemsPlane->childItems()) {
    if (dynamic_cast<PropagatingGraphicsObject*>(item)) {
      dynamic_cast<PropagatingGraphicsObject*>(item)->setImgTransform(t);
    } else {
      item->setPos(t.map(item->pos()));
    }
  }
  if (imageData)
    imageData->addTransform(t.inverted());
  //TODO: change detector size
}

void Projector::enableProjection(bool b) {
  projectionEnabled=b;
}

void Projector::projector2xml(QXmlStreamWriter& w) {
  w.writeEmptyElement("QRange");
  w.writeAttribute("Qmin", QString::number(Qmin()));
  w.writeAttribute("Qmax", QString::number(Qmax()));

  w.writeEmptyElement("Display");
  w.writeAttribute("maxHKLSum", QString::number(getMaxHklSqSum()));
  w.writeAttribute("textSize", QString::number(getTextSize()));
  w.writeAttribute("spotSize", QString::number(getSpotSize()));
  if (spotsEnabled())
    w.writeAttribute("spotsEnabled", "1");

  w.writeStartElement("SpotMarkers");
  for (int n=0; n<spotMarkers().size(); n++) {
    QPointF p=getSpotMarkerDetPos(n);
    w.writeEmptyElement("Spot");
    w.writeAttribute("x", QString::number(p.x()));
    w.writeAttribute("y", QString::number(p.y()));
  }
  w.writeEndElement();


}

bool Projector::loadFromXML(QDomElement base) {
  if (base.tagName()!="Projector") return false;
  for (QDomElement e=base.firstChildElement(); !e.isNull(); e=e.nextSiblingElement()) {
    if (!parseXMLEmelemt(e)) return false;
  }
  return true;
}

bool Projector::parseXMLEmelemt(QDomElement e) {
  bool ok;
  if (e.tagName()=="QRange") {
    double Qmin = e.attribute("Qmin").toDouble(&ok); if (!ok) return false;
    double Qmax = e.attribute("Qmax").toDouble(&ok); if (!ok) return false;
    setWavevectors(Qmin, Qmax);
  } else if (e.tagName()=="Display") {
    double tsize = e.attribute("textSize").toDouble(&ok); if (!ok) return false;
    double ssize = e.attribute("spotSize").toDouble(&ok); if (!ok) return false;
    int maxHKLS = e.attribute("maxHKLSum").toInt(&ok); if (!ok) return false;
    int senabled = e.attribute("spotsEnabled").toInt(&ok); if (!ok) return false;
    setTextSizeFraction(tsize);
    setSpotSizeFraction(ssize);
    setMaxHklSqSum(maxHKLS);
    enableSpots(senabled!=0);
  } else {
    return false;
  }
  return true;
}


int QPointFVector_ID = qRegisterMetaType<QVector<QPointF> >("QVector<QPointF>");
int QGraphicsItemList_ID = qRegisterMetaType<QList<QGraphicsItem*> >("QList<QGraphicsItem*>");

