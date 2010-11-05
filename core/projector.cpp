#include <core/projector.h>
#include <cmath>
#include <iostream>
#include <QtCore/QTimer>
#include <QtGui/QCursor>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QWidget>
#include <QThreadPool>
#include <QtConcurrentMap>
#include <QGraphicsView>
#include <tools/signalingellipse.h>
#include <tools/ruleritem.h>
#include <tools/zoneitem.h>
#include <core/reflection.h>
#include <core/crystal.h>


using namespace std;

Projector::Projector(QObject *parent):
    QObject(parent),
    FitObject(),
    decorationItems(),
    textMarkerItems(),
    spotMarkerItems(),
    zoneMarkerItems(),
    rulerItems(),
    crystal(),
    scene(this),
    spotMarkers(new SpotMarkerGraphicsItem())
{
  scene.addItem(spotMarkers);


  imgGroup = new QGraphicsPixmapItem();
  imgGroup->setFlag(QGraphicsItem::ItemIsMovable, false);
  //imgGroup->setFlag(QGraphicsItem::ItemHasNoContents, true);
  scene.addItem(imgGroup);
  imgGroup->stackBefore(spotMarkers);


  //QPixmap pix("../Clip4/Silver.jpg");
  //img = new QGraphicsPixmapItem(pix, imgGroup);
  //img->setTransform(QTransform::fromScale(1.0/pix.width(), 1.0/pix.height()));
  //img->setTransformationMode(Qt::SmoothTransformation);

  enableSpots();
  enableProjection();
  updateImgTransformations();

  internalSetWavevectors(0.0, 1.0*M_1_PI);
  setMaxHklSqSum(3);
  setTextSize(4.0);
  setSpotSize(4.0);

  QTimer::singleShot(0, this, SLOT(decorateScene()));
  connect(this, SIGNAL(projectionParamsChanged()), this, SLOT(reflectionsUpdated()));
  connect(&scene, SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(updateImgTransformations()));
  connect(&rulerMapper, SIGNAL(mapped(int)), this, SIGNAL(rulerChanged(int)));

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

QString formatOveline(int i) {
  if (i<0)
    return QString("<span style=""text-decoration:overline"">%1</span>").arg(-i);
  return QString("<span>%1</span>").arg(i);
}

QString formatHklText(int h, int k, int l) {
  if (h<10 and k<10 and l<10) {
    return formatOveline(h)+formatOveline(k)+formatOveline(l);
  } else {
    return formatOveline(h)+" "+formatOveline(k)+" "+formatOveline(l);
  }
}

void Projector::addInfoItem(const QString& text, const QPointF& p) {
  QGraphicsRectItem* bg=new QGraphicsRectItem();
  bg->setPen(QPen(Qt::black));
  bg->setBrush(QBrush(QColor(0xF0,0xF0,0xF0)));
  bg->setPos(p);
  bg->setFlag(QGraphicsItem::ItemIsMovable, true);
  bg->setCursor(QCursor(Qt::SizeAllCursor));
  bg->setZValue(1);
  QGraphicsTextItem*  t = new QGraphicsTextItem(bg);
  t->setHtml(text);
  QRectF r=t->boundingRect();
  double sx=textSize*scene.width()/r.width();
  double sy=textSize*scene.height()/r.height();
  double s=sx<sy?sy:sx;

  bg->setRect(t->boundingRect());
  bg->scale(s,s);
  scene.addItem(bg);
  infoItems.append(bg);
}

void Projector::clearInfoItems() {
  while (infoItems.size()>0) {
    QGraphicsItem* item=infoItems.takeLast();
    scene.removeItem(item);
    delete item;
  }
}

Projector::ProjectionMapper::ProjectionMapper(Projector *p, QVector<Reflection> r):
    projector(p),
    reflections(r),
    nextReflection(0),
    nextUnusedPoint(0),
    mutex() {
  if (projector->spotMarkers->coordinates.size()!=r.size())
    projector->spotMarkers->coordinates.resize(r.size());
  setAutoDelete(true);
}

Projector::ProjectionMapper::~ProjectionMapper() {
  projector->spotMarkers->paintUntil = nextUnusedPoint;
}

void Projector::ProjectionMapper::run() {
  QThreadPool::globalInstance()->tryStart(this);
  int i;
  while ((i=nextReflection.fetchAndAddAcquire(1))<reflections.size()) {
    QPointF p;
    const Reflection& r = reflections.at(i);
    if (projector->project(r, p)) {
      projector->spotMarkers->coordinates[nextUnusedPoint.fetchAndAddOrdered(1)]=p;

      if (r.hklSqSum<=(projector->maxHklSqSum)) {
        QGraphicsTextItem* t = new QGraphicsTextItem();
        t->setTransform(QTransform(1,0,0,-1,0,0));
        t->setHtml(formatHklText(r.h, r.k, r.l));
        t->setPos(p);
        QRectF r=t->boundingRect();
        double sx=projector->textSize*projector->scene.width()/r.width();
        double sy=projector->textSize*projector->scene.height()/r.height();
        double s=sx<sy?sy:sx;
        t->scale(s,s);
        mutex.lock();
        projector->textMarkerItems.append(t);
        mutex.unlock();
      }

    }
  }
}


void Projector::reflectionsUpdated() {
  if (crystal.isNull() or not projectionEnabled)
    return;

  foreach (QGraphicsItem* item, textMarkerItems) {
    scene.removeItem(item);
    delete item;
  }
  textMarkerItems.clear();

  clearInfoItems();

  spotMarkers->setSpotsize(spotSize);
  QThreadPool::globalInstance()->start(new ProjectionMapper(this, crystal->getReflectionList()));
  QThreadPool::globalInstance()->waitForDone();
  spotMarkers->pointsUpdated();
  foreach (QGraphicsItem* item, textMarkerItems) {
    //It is not possiple to add the TextItem in the thread.
    scene.addItem(item);
  }
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

Crystal* Projector::getCrystal() {
  return crystal;
}

int Projector::getMaxHklSqSum() const {
  return maxHklSqSum;
}

double Projector::getSpotSize() const {
  return 100.0*spotSize;
}

double Projector::getTextSize() const {
  return 100.0*textSize;
}

bool Projector::spotsEnabled() const {
  return showSpots;
}

void Projector::setMaxHklSqSum(int m) {
  maxHklSqSum=m;
  emit projectionParamsChanged();
}

void Projector::setTextSize(double d) {
  textSize=0.01*d;
  emit projectionParamsChanged();
}

void Projector::setSpotSize(double d) {
  spotSize=0.01*d;
  emit projectionParamsChanged();
}

void Projector::enableSpots(bool b) {
  showSpots=b;
  spotMarkers->setVisible(b);
  emit projectionParamsChanged();
}

void Projector::addSpotMarker(const QPointF& p) {
  QRectF r(-spotSize, -spotSize, 2.0*spotSize, 2.0*spotSize);

  SignalingEllipseItem* marker=new SignalingEllipseItem(imgGroup);
  marker->setFlag(QGraphicsItem::ItemIsMovable, true);
  marker->setCursor(QCursor(Qt::SizeAllCursor));
  marker->setPen(QPen(QColor(0xFF,0xAA,0x33)));
  marker->setRect(r);
  marker->setPos(det2img.map(p));
  marker->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  connect(marker, SIGNAL(positionChanged()), this, SIGNAL(spotMarkerChanged()));
  spotMarkerItems.append(marker);
  emit spotMarkerAdded();
};

void Projector::delSpotMarkerNear(const QPointF& p) {
  if (spotMarkerItems.isEmpty())
    return;
  double minDist=0.0;
  int minIdx=-1;
  QGraphicsEllipseItem* m;
  QPointF imgPos=det2img.map(p);
  for (int i=0; i<spotMarkerItems.size(); i++) {
    m=spotMarkerItems.at(i);
    double d=hypot(imgPos.x()-m->pos().x(), imgPos.y()-m->pos().y());
    if (i==0 or d<minDist) {
      minDist=d;
      minIdx=i;
    }
  }
  m=spotMarkerItems.takeAt(minIdx);
  scene.removeItem(m);
  delete m;
};

int Projector::spotMarkerNumber() const {
  return spotMarkerItems.size();
}

QPointF Projector::getSpotMarkerDetPos(int n) const {
  return img2det.map(spotMarkerItems.at(n)->pos());
}

QList<Vec3D> Projector::getSpotMarkerNormals() const {
  QList<Vec3D> r;
  foreach (QGraphicsItem* item, spotMarkerItems)
    r << det2normal(img2det.map(item->pos()));
  return r;
}

// ---------------  Ruler handling ---------------------------
int Projector::rulerNumber() const {
  return rulerItems.size();
}

void Projector::addRuler(const QPointF& p1, const QPointF& p2) {
  RulerItem* ruler = new RulerItem(det2img.map(p1), det2img.map(p2), this, imgGroup);
  ruler->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  rulerMapper.setMapping(ruler, rulerItems.size());
  connect(ruler, SIGNAL(rulerChanged()), &rulerMapper, SLOT(map()));
  rulerItems << ruler;
  emit rulerAdded();
}

void Projector::clearRulers() {
  foreach (RulerItem* item, rulerItems) {
    scene.removeItem(item);
    delete item;
  }
  rulerItems.clear();
}

QPair<QPointF, QPointF> Projector::getRulerCoordinates(int n) {
  if (n<rulerItems.size()) {
    return qMakePair(img2det.map(rulerItems.at(n)->getStart()), img2det.map(rulerItems.at(n)->getEnd()));
  }
  return QPair<QPointF, QPointF>();
}

void Projector::highlightRuler(int n, bool b) {
  if (n<rulerItems.size()) {
    rulerItems.at(n)->highlight(b);
  }
}

bool Projector::rulerIsHighlighted(int n) {
  if (n<rulerItems.size()) {
    return rulerItems.at(n)->isHighlighted();
  }
  return false;
}

QVariant Projector::getRulerData(int n) {
  if (n<rulerItems.size()) {
    return rulerItems.at(n)->data(0);
  }
  return QVariant();
}

void Projector::setRulerData(int n, QVariant v) {
  if (n<rulerItems.size()) {
    rulerItems[n]->setData(0, v);
  }
}

// ----------------------- Handling of Zone Markers -------------
int Projector::zoneMarkerNumber() const {
  return zoneMarkerItems.size();
}

void Projector::addZoneMarker(const QPointF& p1, const QPointF& p2) {
  ZoneItem* zoneMarker = new ZoneItem(det2img.map(p1), det2img.map(p2), this, imgGroup);
  zoneMarker->setTransform(QTransform::fromScale(det2img.m11(), det2img.m22()));
  connect(this, SIGNAL(spotMarkerAdded()), zoneMarker, SLOT(updateOptimalZone()));
  connect(this, SIGNAL(spotMarkerChanged()), zoneMarker, SLOT(updateOptimalZone()));
  zoneMarkerItems << zoneMarker;
  emit zoneMarkerAdded();
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
  imgGroup->setTransform(img2det);
  QTransform t = QTransform::fromScale(det2img.m11(), det2img.m22());
  foreach (QGraphicsItem* item, imgGroup->childItems()) {
    if (item != img)
      item->setTransform(t);
  }
  emit imgTransformUpdated();
}

// Rotates and flips the Decorations, which are bound to the Image
void Projector::doImgRotation(int CWRSteps, bool flip) {
  QTransform t;
  foreach (QGraphicsItem* item, imgGroup->childItems()) {
    double x = item->pos().x();
    double y = item->pos().y();

    if (flip) x=1.0-x;
    double t;
    switch(CWRSteps) {
    case 1:
      t=x;
      x=y;
      y=1.0-t;
      break;
    case 2:
      x=1.0-x;
      y=1.0-y;
      break;
    case 3:
      t=x;
      x=1.0-y;
      y=t;
      break;
    }
    item->setPos(x,y);
  }
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
  for (int n=0; n<spotMarkerNumber(); n++) {
    QPointF p=getSpotMarkerDetPos(n);
    w.writeEmptyElement("Spot");
    w.writeAttribute("x", QString::number(p.x()));
    w.writeAttribute("y", QString::number(p.y()));
  }
  w.writeEndElement();


}

void Projector::loadFromXML(QXmlStreamReader &r) {
  if (not (r.name()=="Projector" and r.isStartElement()))
    return;
  while (not (r.atEnd() or (r.name()=="Projector" and r.isEndElement()))) {
    r.readNext();
    if (r.isStartElement())
      if (not parseXMLElement(r)) {
      cout << "Could not parse: " << qPrintable(r.name().toString()) << endl;
    }
  }
}


double getDoubleAttrib(QXmlStreamReader &r, QString name, double def) {
  bool b;
  QStringRef sr = r.attributes().value(name);
  if (not sr.isNull()) {
    double v = sr.toString().toDouble(&b);
    if (b)
      return v;
  }
  return def;
}

int getIntAttrib(QXmlStreamReader &r, QString name, int def) {
  bool b;
  QStringRef sr = r.attributes().value(name);
  if (not sr.isNull()) {
    int v =sr.toString().toInt(&b);
    if (b)
      return v;
  }
  return def;
}

bool Projector::parseXMLElement(QXmlStreamReader &r) {
  if (r.name()=="QRange") {
    double qMin=getDoubleAttrib(r, "Qmin", Qmin());
    double qMax=getDoubleAttrib(r, "Qmax", Qmax());
    setWavevectors(qMin, qMax);
    return true;
  } else if (r.name()=="Display") {
    setMaxHklSqSum(getIntAttrib(r, "maxHKLSum", (int)getMaxHklSqSum()));
    setSpotSize(getDoubleAttrib(r, "spotSize", getSpotSize()));
    setTextSize(getDoubleAttrib(r, "textSize", getTextSize()));
    QStringRef sr = r.attributes().value("spotsEnabled");
    enableSpots(not sr.isNull());
    return true;
  } else if (r.name()=="SpotMarkers") {
    while (not (r.atEnd() or (r.name()=="SpotMarkers" and r.isEndElement()))) {
      r.readNext();
      if (r.isStartElement() and r.name()=="Spot") {
        bool b1, b2;
        double x = r.attributes().value("x").toString().toDouble(&b1);
        double y = r.attributes().value("y").toString().toDouble(&b2);
        if (b1 and b2) {
          addSpotMarker(QPointF(x,y));
        }
      }
    }
    return true;
  }
  return false;
}



Projector::SpotMarkerGraphicsItem::SpotMarkerGraphicsItem(): QGraphicsItem(), workerSync(0) {
  setCacheMode(NoCache);
  cacheNeedsUpdate = true;
  for (int i=0; i<QThread::idealThreadCount(); i++) {
    Worker* w = new Worker(this, i);
    w->start();
    workers << w;
  }
};

Projector::SpotMarkerGraphicsItem::~SpotMarkerGraphicsItem() {
  for (int i=0; i<workers.size(); i++) {
    workers.at(i)->shouldStop=true;
  }
  workerStart.wakeAll();
  workerSync.acquire(workers.size());
  for (int i=0; i<workers.size(); i++) {
    delete workers[i];
  }
  workers.clear();

}

void Projector::SpotMarkerGraphicsItem::updateCache() {
  if (cacheNeedsUpdate) {
    workN = 0;
    workerStart.wakeAll();
    cache.fill(QColor(0,0,0,0));
    workerSync.acquire(workers.size());

    QPainter p2(&cache);
    foreach (Worker* worker, workers) {
      p2.drawImage(QPoint(0,0), *worker->localCache);
    }
    cacheNeedsUpdate=false;
  }
}

void Projector::SpotMarkerGraphicsItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w) {  

  if (cache.size()!=p->viewport().size()) {
    cache = QPixmap(p->viewport().size());
    cacheNeedsUpdate = true;
  }

  if (transform!=p->worldTransform()) {
    transform = p->worldTransform();
    cacheNeedsUpdate = true;
  }

  updateCache();

  p->save();
  p->resetTransform();
  p->drawPixmap(QPoint(0,0), cache);
  p->restore();


}

void Projector::SpotMarkerGraphicsItem::pointsUpdated() {
  cacheNeedsUpdate = true;
  prepareGeometryChange();
}

QRectF Projector::SpotMarkerGraphicsItem::boundingRect() const {
  if (coordinates.size()>0) {
    QRectF r(coordinates.at(0), QSizeF(0,0));
    for (int i=1; i<paintUntil; i++) {
      QPointF p(coordinates.at(i));
      r.setLeft(qMin(p.x(), r.left()));
      r.setRight(qMax(p.x(), r.right()));
      r.setTop(qMin(p.y(), r.top()));
      r.setBottom(qMax(p.y(), r.bottom()));
    }
    return r;
  } else {
    return QRectF();
  }
}


void Projector::SpotMarkerGraphicsItem::Worker::run() {
  forever {
    spotMarker->mutex.lock();
    spotMarker->workerStart.wait(&spotMarker->mutex);
    spotMarker->mutex.unlock();

    if (shouldStop) {
      spotMarker->workerSync.release(1);
      delete localCache;
      return;
    }

    double rx = spotMarker->transform.m11()*spotMarker->spotSize;
    double ry = spotMarker->transform.m22()*spotMarker->spotSize;


    if (!localCache || localCache->size()!=spotMarker->cache.size())
      localCache = new QImage(spotMarker->cache.size(), QImage::Format_ARGB32_Premultiplied);
    localCache->fill(QColor(0,0,0,0).rgba());
    QPainter painter(localCache);
    QList<QGraphicsView*> l = spotMarker->scene()->views();
    if (l.size())
      painter.setRenderHints(l.at(0)->renderHints());
    painter.setPen(Qt::green);
    int i;
    while ((i=spotMarker->workN.fetchAndAddOrdered(1))<spotMarker->paintUntil) {
      painter.drawEllipse(spotMarker->transform.map(spotMarker->coordinates.at(i)), rx, ry);

    }
    painter.end();
    spotMarker->workerSync.release(1);

  }

}
