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
#include <QSettings>

#include "tools/circleitem.h"
#include "tools/ruleritem.h"
#include "tools/spotitem.h"
#include "tools/zoneitem.h"
#include "tools/cropmarker.h"
#include "core/reflection.h"
#include "core/crystal.h"
#include "image/laueimage.h"
#include "tools/spotindicatorgraphicsitem.h"
#include "tools/xmltools.h"
#include "config/configstore.h"

using namespace std;

Projector::Projector(QObject *parent):
    FitObject(parent),
    decorationItems(),
    textMarkerItems(),
    spotMarkerStore(this),
    zoneMarkerStore(this),
    rulerStore(this),
    infoStore(this),
    crystal(),
    scene(this),
    imageItemsPlane(new QGraphicsPixmapItem()),
    spotIndicator(new SpotIndicatorGraphicsItem()),
    imageData(0),
    spotHighlightHKL(),
    spotHighlightItem(0)
{
  imageItemsPlane->setFlag(QGraphicsItem::ItemIsMovable, false);

  scene.addItem(spotIndicator);
  scene.addItem(imageItemsPlane);
  scene.setItemIndexMethod(QGraphicsScene::NoIndex);

  spotIndicator->stackBefore(imageItemsPlane);
  spotIndicator->setCachedPainting(false);

  enableSpots();
  enableMarkers();
  setHQPrintMode(false);
  enableProjection();
  updateImgTransformations();

  internalSetWavevectors(0.0, 1.0*M_PI);
  setMaxHklSqSum(3);
  setTextSizeFraction(10.0);
  setSpotSizeFraction(1.0);

  connect(this, SIGNAL(spotSizeChanged(double)), this, SLOT(doProjection()));
  connect(this, SIGNAL(textSizeChanged(double)), this, SLOT(doProjection()));

  QTimer::singleShot(0, this, SLOT(decorateScene()));
  connect(this, SIGNAL(projectionParamsChanged()), this, SLOT(invalidateMarkerCache()));
  connect(this, SIGNAL(projectionParamsChanged()), this, SLOT(doProjection()));
  connect(&scene, SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(updateImgTransformations()));

  connect(&spotMarkerStore, SIGNAL(itemAdded(int)), this, SLOT(spotMarkerAdded(int)));
  connect(&spotMarkerStore, SIGNAL(itemChanged(int)), this, SLOT(spotMarkerChanged(int)));
  connect(&spotMarkerStore, SIGNAL(itemAboutToBeRemoved(int)), this, SLOT(spotMarkerRemoved(int)));
  connect(&spotMarkerStore, SIGNAL(itemClicked(int)), this, SLOT(spotMarkerClicked(int)));
  connect(&zoneMarkerStore, SIGNAL(itemAdded(int)), this, SLOT(zoneMarkerAdded(int)));
  connect(&zoneMarkerStore, SIGNAL(itemChanged(int)), this, SLOT(zoneMarkerChanged(int)));
  connect(&zoneMarkerStore, SIGNAL(itemAboutToBeRemoved(int)), this, SLOT(zoneMarkerRemoved(int)));
  connect(&zoneMarkerStore, SIGNAL(itemClicked(int)), this, SLOT(zoneMarkerClicked(int)));
}


Projector::~Projector() {
  if (!crystal.isNull())
    crystal->removeProjector(this);
}

QString Projector::FitObjectName() {
  return displayName();
}

Projector& Projector::operator=(const Projector& o) {
  det2img = o.det2img;
  img2det = o.img2det;

  internalSetWavevectors(o.Qmin(), o.Qmax());
  setMaxHklSqSum(o.getMaxHklSqSum());
  setTextSizeFraction(o.getTextSizeFraction());
  setSpotSizeFraction(o.getSpotSizeFraction());
  enableSpots(o.spotsEnabled());
  enableProjection((o.isProjectionEnabled()));

  foreach (SpotItem* si, o.spotMarkerStore) {
    addSpotMarker(img2det.map(si->pos()));
  }

  foreach (ZoneItem* zi, o.zoneMarkerStore) {
    addZoneMarker(o.img2det.map(zi->getStart()), o.img2det.map(zi->getEnd()));
  }

  FitObject::operator =(o);

  return *this;
}

void Projector::connectToCrystal(Crystal *c) {
  if (!crystal.isNull()) {
    disconnect(c);
    crystal->removeProjector(this);
  }
  crystal=c;
  crystal->addProjector(this);
  connect(crystal, SIGNAL(reflectionsUpdate()), this, SLOT(doProjection()));
  connect(crystal, SIGNAL(cellChanged()), this, SLOT(invalidateMarkerCache()));
  connect(crystal, SIGNAL(orientationChanged()), this, SLOT(invalidateMarkerCache()));
  connect(crystal, SIGNAL(deleteMarker(AbstractMarkerItem*)), this, SLOT(deleteMarker(AbstractMarkerItem*)));
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
  QminVal=Qmin;
  QmaxVal=Qmax;
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

ItemStore<QGraphicsRectItem>& Projector::infoItems() {
  return infoStore;
}

void Projector::doProjection() {
  if (crystal.isNull() or !isProjectionEnabled())
    return;

  // Remove the textmarkeritems from the scene
  foreach (QGraphicsItem* item, textMarkerItems) {
    scene.removeItem(item);
    delete item;
  }
  textMarkerItems.clear();

  // Clear the coordinates of the old projected spots
  spotIndicator->coordinates.clear();
  // Load Reflection List
  QVector<Reflection> refs = crystal->getReflectionList();
  // Resize the array, that caches the Information if a reflection is actually projected
  reflectionIsProjected.resize(refs.size());

  QPointF p;
  // Loop over all reflections
  for (int i=0; i<refs.size(); i++) {
    // Do the actual projection and store, if reflection could be projected
    if ((reflectionIsProjected[i] = project(refs[i], p))) {
      // Save the projected coordinate
      spotIndicator->coordinates << p;
      // If hklSum is below limit, add a textitem to the scene
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

  updateSpotHighlightMarker();

  // set the spotsize (just in case)
  spotIndicator->setSpotsize(getSpotSize());
  // indicate that the coordinates have changed.
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
  if (r.size()!=reflectionIsProjected.size()) cout << "Something horrible is going on..." << endl;
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

QList<Reflection> Projector::getProjectedReflections() {
  QList<Reflection> list;
  QVector<Reflection> r = crystal->getReflectionList();
  for (int n=0; n<r.size(); n++)
    if (reflectionIsProjected.at(n))
      list << r.at(n);
  return list;
}

QList<Reflection> Projector::getProjectedReflectionsNormalToZone(const TVec3D<int>& uvw) {
  QList<Reflection> list;
  QVector<Reflection> r = crystal->getReflectionList();
  for (int n=0; n<r.size(); n++)
    if (reflectionIsProjected.at(n) && ((r.at(n).hkl()*uvw)==0))
      list << r.at(n);
  return list;
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

void Projector::invalidateMarkerCache() {
  foreach (AbstractMarkerItem* im, getAllMarkers()) im->invalidateCache();
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
  emit spotSizeChanged(getSpotSize());
}

void Projector::enableSpots(bool b) {
  spotIndicator->setVisible(b);
  emit projectionParamsChanged();
}

bool Projector::spotsEnabled() const {
  return spotIndicator->isVisible();
}

void Projector::enableMarkers(bool b) {
  showMarkers = b;
  foreach (SpotItem* si, spotMarkers()) si->setVisible(showMarkers);
  foreach (ZoneItem* zi, zoneMarkers()) zi->setVisible(showMarkers);
  emit projectionParamsChanged();
}

bool Projector::markersEnabled() const {
  return showMarkers;
}

void Projector::enableProjection(bool b) {
  projectionEnabled=b;
}

bool Projector::isProjectionEnabled() const {
  return projectionEnabled;
}

void Projector::setHQPrintMode(bool b) {
  spotIndicator->setCachedPainting(!b);
}

void Projector::setSpotHighlighting(Vec3D hkl) {
  //if (spotHighlightHKL != hkl ) {
    spotHighlightHKL = hkl;
    updateSpotHighlightMarker();
  //}
}

void Projector::updateSpotHighlightMarker() {
  bool show = (!crystal.isNull() && !spotHighlightHKL.isNull());
  QPointF p;
  if (show) {
    Vec3D v = crystal->hkl2Reziprocal(spotHighlightHKL);
    p = normal2det(v.normalized(), show);
    //TODO: emit including scattering orders
    emit spotHighlightChanged(v);
  }

  if (show) {
    if (spotHighlightItem==0) {
      CircleItem* item = new CircleItem(1.1*getSpotSize());
      ConfigStore::getInstance()->ensureColor(ConfigStore::SpotIndicatorHighlight, item, SLOT(setColor(QColor)));
      item->setLineWidth(3);
      spotHighlightItem = item;
      scene.addItem(spotHighlightItem);
    }
    spotHighlightItem->setPos(p);

  } else {
    delete spotHighlightItem;
    spotHighlightItem = 0;
  }
}

// ----------------------- Handling of Spot Markers -------------
void Projector::addSpotMarker(const QPointF& p) {
  SpotItem* item = new SpotItem(this, getSpotSize(), imageItemsPlane);
  item->setVisible(showMarkers);
  connect(this, SIGNAL(spotSizeChanged(double)), item, SLOT(setRadius(double)));
  item->setPos(det2img.map(p));
  item->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  spotMarkers().addItem(item);
}

ItemStore<SpotItem>& Projector::spotMarkers() {
  return spotMarkerStore;
}

QPointF Projector::getSpotMarkerDetPos(int n) {
  return img2det.map(spotMarkers().at(n)->pos());
}

QList<Vec3D> Projector::getSpotMarkerNormals() {
  QList<Vec3D> r;
  foreach (SpotItem* si, spotMarkers())
    r << si->getMarkerNormal();
  return r;
}

// ----------------------- Handling of Zone Markers -------------

void Projector::addZoneMarker(const QPointF& p1, const QPointF& p2) {
  ZoneItem* zoneMarker = new ZoneItem(det2img.map(p1), det2img.map(p2), this, imageItemsPlane);
  zoneMarker->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  zoneMarker->setVisible(showMarkers);
  connect(&spotMarkers(), SIGNAL(itemAdded(int)), zoneMarker, SLOT(updateOptimalZone()));
  connect(&spotMarkers(), SIGNAL(itemChanged(int)), zoneMarker, SLOT(updateOptimalZone()));
  connect(&spotMarkers(), SIGNAL(itemRemoved(int)), zoneMarker, SLOT(updateOptimalZone()));
  zoneMarkers().addItem(zoneMarker);
}

QList<Vec3D> Projector::getZoneMarkerNormals() {
  QList<Vec3D> r;
  foreach (ZoneItem* zi, zoneMarkers())
    r << zi->getMarkerNormal();
  return r;
}

ItemStore<ZoneItem>& Projector::zoneMarkers() {
  return zoneMarkerStore;
}

// ---------------  General FitMarker Handling--------------------

bool Projector::hasMarkers() {
  return (spotMarkers().size()>0) || (zoneMarkers().size()>0);
}

QList<AbstractMarkerItem*> Projector::getAllMarkers() {
  QList<AbstractMarkerItem*> list;
  foreach (SpotItem* si, spotMarkers()) list << si;
  foreach (ZoneItem* zi, zoneMarkers()) list << zi;
  return list;
}

void Projector::spotMarkerAdded(int n) {
  emit markerAdded(spotMarkerStore.at(n));
}

void Projector::spotMarkerChanged(int n) {
  emit markerChanged(spotMarkerStore.at(n));
}

void Projector::spotMarkerClicked(int n) {
  emit markerClicked(spotMarkerStore.at(n));
}

void Projector::spotMarkerRemoved(int n) {
  emit markerRemoved(spotMarkerStore.at(n));
}

void Projector::zoneMarkerAdded(int n) {
  emit markerAdded(zoneMarkerStore.at(n));
}

void Projector::zoneMarkerChanged(int n) {
  emit markerChanged(zoneMarkerStore.at(n));
}

void Projector::zoneMarkerClicked(int n) {
  emit markerClicked(zoneMarkerStore.at(n));
}

void Projector::zoneMarkerRemoved(int n) {
  emit markerRemoved(zoneMarkerStore.at(n));
}
void Projector::deleteMarker(AbstractMarkerItem* item) {
  if (ZoneItem* zi=dynamic_cast<ZoneItem*>(item)) {
    zoneMarkers().del(zi);
  } else if (SpotItem* si=dynamic_cast<SpotItem*>(item)) {
    spotMarkers().del(si);
  }
}

// ---------------  Ruler handling ---------------------------
ItemStore<RulerItem>& Projector::rulers() {
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
    RulerItem* ruler = rulers().at(n);
    if (ruler)
      return qMakePair(img2det.map(ruler->getStart()), img2det.map(ruler->getEnd()));
  }
  return QPair<QPointF, QPointF>();
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
  delCropMarker();
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
  LaueImage* tmpImage = new LaueImage(this);
  connect(tmpImage, SIGNAL(openFinished(LaueImage*)), this, SLOT(setImage(LaueImage*)));
  tmpImage->startOpenFile(s);
}

void Projector::setImage(LaueImage *tmpImage) {
  if (tmpImage->isValid()) {
    closeImage();
    imageData = tmpImage;
    emit imageLoaded(imageData);
    connect(imageData, SIGNAL(imageContentsChanged()), getScene(), SLOT(update()));
  } else {
    tmpImage->deleteLater();
  }
}

void Projector::closeImage() {
  if (imageData) {
    delete imageData;
    imageData = 0;
    emit imageClosed();
  }
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
}



const char XML_Projector_element[] = "Projector";
const char XML_Projector_QRange[] = "QRange";
const char XML_Projector_QRange_min[] = "Qmin";
const char XML_Projector_QRange_max[] = "Qmax";
const char XML_Projector_Display[] = "Display";
const char XML_Projector_Display_maxHKL[] = "maxHKLSum";
const char XML_Projector_Display_spotSize[] = "spotSize";
const char XML_Projector_Display_textSize[] = "textSize";
const char XML_Projector_Display_spotsEnables[] = "spotsEnabled";
const char XML_Projector_SpotMarkers[] = "SpotMarkers";
const char XML_Projector_SpotMarkers_marker[] = "Marker";
const char XML_Projector_ZoneMarkers[] = "ZoneMarkers";

QDomElement Projector::saveToXML(QDomElement base) {
  QDomDocument doc = base.ownerDocument();
  QDomElement projector = ensureElement(base, XML_Projector_element);

  QDomElement e = projector.appendChild(doc.createElement(XML_Projector_QRange)).toElement();
  e.setAttribute(XML_Projector_QRange_min, Qmin());
  e.setAttribute(XML_Projector_QRange_max, Qmax());

  e = projector.appendChild(doc.createElement(XML_Projector_Display)).toElement();
  e.setAttribute(XML_Projector_Display_maxHKL, getMaxHklSqSum());
  e.setAttribute(XML_Projector_Display_textSize, getTextSizeFraction());
  e.setAttribute(XML_Projector_Display_spotSize, getSpotSizeFraction());
  e.setAttribute(XML_Projector_Display_spotsEnables, spotsEnabled());

  if (spotMarkers().size()>0) {
    e = projector.appendChild(doc.createElement(XML_Projector_SpotMarkers)).toElement();
    foreach (QGraphicsItem* item, spotMarkers()) {
      PointToTag(e, XML_Projector_SpotMarkers_marker, item->pos());
    }
  }

  if (zoneMarkers().size()>0) {
    e = projector.appendChild(doc.createElement(XML_Projector_ZoneMarkers)).toElement();
    foreach (ZoneItem* zi, zoneMarkers()) {
      zi->saveToXML(e);
    }
  }

  if (getLaueImage()) {
    getLaueImage()->saveToXML(projector);
  }

  return projector;
}

bool Projector::loadFromXML(QDomElement base) {
  QDomElement element = base.elementsByTagName(XML_Projector_element).at(0).toElement();
  if (element.isNull()) return false;
  for (QDomElement e=element.firstChildElement(); !e.isNull(); e=e.nextSiblingElement()) {
    if (!parseXMLElement(e)) {
      cout << "Could not parse: " << qPrintable(e.tagName()) << endl;
    }
  }

  LaueImage* tmpImageData = new LaueImage();
  connect(tmpImageData, SIGNAL(openFinished(LaueImage*)), this, SLOT(setImage(LaueImage*)));
  connect(tmpImageData, SIGNAL(openFailed(LaueImage*)), tmpImageData, SLOT(deleteLater()));
  tmpImageData->loadFromXML(base);

  return true;
}

bool Projector::parseXMLElement(QDomElement e) {
  bool ok=true;
  if (e.tagName()==XML_Projector_QRange) {
    double Qmin = readDouble(e, XML_Projector_QRange_min, ok);
    double Qmax = readDouble(e, XML_Projector_QRange_max, ok);
    if (ok) setWavevectors(Qmin, Qmax);
  } else if (e.tagName()=="Display") {
    double tsize = readDouble(e, XML_Projector_Display_textSize, ok);
    double ssize = readDouble(e, XML_Projector_Display_spotSize, ok);
    int maxHKLS = readInt(e, XML_Projector_Display_maxHKL, ok);
    int senabled = readInt(e, XML_Projector_Display_spotsEnables, ok);
    if (ok) {
      setTextSizeFraction(tsize);
      setSpotSizeFraction(ssize);
      setMaxHklSqSum(maxHKLS);
      enableSpots(senabled!=0);
    }
  } else if (e.tagName()==XML_Projector_SpotMarkers) {
    for (QDomElement m=e.firstChildElement(); !m.isNull(); m=m.nextSiblingElement()) {
      addSpotMarker(img2det.map(TagToPoint(m, QPointF(), &ok)));
    }
  } else if (e.tagName()==XML_Projector_ZoneMarkers) {
    for (QDomElement m=e.firstChildElement(); !m.isNull(); m=m.nextSiblingElement()) {
      addZoneMarker(QPointF(), QPointF());
      zoneMarkers().last()->loadFromXML(m);
    }
  } else {
    return false;
  }
  return ok;
}

void Projector::saveParametersAsDefault() {
  QSettings settings;
  settings.beginGroup(projectorName());
  settings.setValue("Qmin", Qmin());
  settings.setValue("Qmax", Qmax());
  settings.setValue("maxHKLSqSum", getMaxHklSqSum());
  settings.setValue("textSizeFraction", getTextSizeFraction());
  settings.setValue("spotSizeFraction", getSpotSizeFraction());
  settings.endGroup();
  emit projectorSavesDefault();
}

int QPointFVector_ID = qRegisterMetaType<QVector<QPointF> >("QVector<QPointF>");
int QGraphicsItemList_ID = qRegisterMetaType<QList<QGraphicsItem*> >("QList<QGraphicsItem*>");

