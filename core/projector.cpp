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

using namespace std;

Projector::Projector(QObject *parent): QObject(parent), FitObject(), decorationItems(), textMarkerItems(), markerItems(), crystal(), scene(this), imgGroup() {
  enableSpots();
  enableProjection();
  scene.setItemIndexMethod(QGraphicsScene::NoIndex);
  internalSetWavevectors(0.0, 1.0*M_1_PI);
  setMaxHklSqSum(0);
  setTextSize(4.0);
  setSpotSize(4.0);
  QTimer::singleShot(0, this, SLOT(decorateScene()));
  connect(this, SIGNAL(projectionParamsChanged()), this, SLOT(reflectionsUpdated()));
  connect(&scene, SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(updateImgTransformations()));
  scene.addItem(&imgGroup);
  imgGroup.setHandlesChildEvents(false);
  //imgGroup.setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  spotMarkers = new Projector::SpotMarkerGraphicsItem();
  scene.addItem(spotMarkers);
  updateImgTransformations();
};

Projector::Projector(const Projector &p): QObject(),FitObject(),det2img(p.det2img), img2det(p.img2det), decorationItems(), textMarkerItems(), markerItems(), infoItems(), crystal(), scene(this)  {
  cout << "Projector Copy Constructor" << endl;
  enableSpots(p.spotsEnabled());
  enableProjection(p.projectionEnabled);
  internalSetWavevectors(p.Qmin(), p.Qmax());
  setMaxHklSqSum(p.getMaxHklSqSum());
  setTextSize(p.getTextSize());
  setSpotSize(p.getSpotSize());

  for (int n=p.markerNumber(); n--; )
    addMarker(p.getMarkerDetPos(n));

  connect(this, SIGNAL(projectionParamsChanged()), this, SLOT(reflectionsUpdated()));
  connect(&scene, SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(updateImgTransformations()));

  spotMarkers = new Projector::SpotMarkerGraphicsItem();
  scene.addItem(spotMarkers);
  updateImgTransformations();
}

Projector::~Projector() {
  delete spotMarkers;
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

Projector::ProjectionMapper::ProjectionMapper(Projector *p, QList<Reflection> r):
    projector(p),
    reflections(r),
    nextReflection(0),
    nextUnusedPoint(0),
    mutex() {
  if (projector->spotMarkers->coordinates.size()!=r.size())
    projector->spotMarkers->coordinates.resize(r.size());
  projector->spotMarkers->cacheNeedsUpdate=true;
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
        mutex.lock();
        QGraphicsTextItem*  t = projector->scene.addText("");
        projector->textMarkerItems.append(t);
        mutex.unlock();
        t->setHtml(formatHklText(r.h, r.k, r.l));
        t->setPos(p);
        QRectF r=t->boundingRect();
        double sx=projector->textSize*projector->scene.width()/r.width();
        double sy=projector->textSize*projector->scene.height()/r.height();
        double s=sx<sy?sy:sx;
        t->scale(s,s);
      }

    }
  }
}


void Projector::reflectionsUpdated() {
  if (crystal.isNull() or not projectionEnabled)
    return;

  while (textMarkerItems.size()>0) {
    QGraphicsItem* item=textMarkerItems.takeLast();
    scene.removeItem(item);
    delete item;
  }

  clearInfoItems();

  spotMarkers->setSpotsize(spotSize);
  QThreadPool::globalInstance()->start(new ProjectionMapper(this, crystal->getReflectionList()));
  QThreadPool::globalInstance()->waitForDone();
  spotMarkers->update();

  emit projectedPointsUpdated();
}


Vec3D Projector::normal2scattered(const Vec3D &v, bool* b) {
  double x=v.x();
  if (x<=0.0) {
    if (b) *b=false;
    return Vec3D();
  }
  double y=v.y();
  double z=v.z();
  if (b) *b=true;
  return Vec3D(2*x*x-1.0, 2.0*x*y, 2.0*x*z);
}

Vec3D Projector::scattered2normal(const Vec3D& v, bool* b) {
  double x=v.x();
  double y=v.y();
  double z=v.z();

  x=sqrt(0.5*(x+1.0));
  if (x==0.0) {
    if (b) *b=false;
    return Vec3D();
  }
  if (b) *b=true;
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
  emit projectionParamsChanged();
}

void Projector::addMarker(const QPointF& p) {
  QRectF r(-0.5*spotSize, -0.5*spotSize, spotSize, spotSize);
  //QRectF r(-5.0,-5.0,10.0,10.0);

  QGraphicsEllipseItem* marker=new QGraphicsEllipseItem(&imgGroup);
  marker->setFlag(QGraphicsItem::ItemIsMovable, true);
  marker->setCursor(QCursor(Qt::SizeAllCursor));
  marker->setPen(QPen(QColor(0xFF,0xAA,0x33)));
  marker->setRect(r);
  marker->setPos(det2img.map(p));
  QTransform t;
  t.scale(det2img.m11(), det2img.m22());
  marker->setTransform(t);

  markerItems.append(marker);
};

void Projector::delMarkerNear(const QPointF& p) {
  if (markerItems.isEmpty())
    return;
  double minDist=0.0;
  int minIdx=-1;
  QGraphicsEllipseItem* m;
  QPointF imgPos=det2img.map(p);
  for (int i=0; i<markerItems.size(); i++) {
    m=markerItems.at(i);
    double d=hypot(imgPos.x()-m->pos().x(), imgPos.y()-m->pos().y());
    if (i==0 or d<minDist) {
      minDist=d;
      minIdx=i;
    }
  }
  m=markerItems.takeAt(minIdx);
  scene.removeItem(m);
  delete m;
};

int Projector::markerNumber() const {
  return markerItems.size();
}

QPointF Projector::getMarkerDetPos(int n) const {
  return img2det.map(markerItems.at(n)->pos());
}

QList<Vec3D> Projector::getMarkerNormals() const {
  QList<Vec3D> r;
  for (int i=0; i<markerItems.size(); i++)
    r << det2normal(img2det.map(markerItems.at(i)->pos()));
  return r;
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
  imgGroup.setTransform(img2det);
  QTransform t;
  t.scale(det2img.m11(), det2img.m22());
  for (int i=imgGroup.childItems().size(); i--; )
    imgGroup.childItems().at(i)->setTransform(t);
  emit imgTransformUpdated();
}

// Rotates and flips the Decorations, which are bound to the Image
void Projector::doImgRotation(int CWRSteps, bool flip) {
  QTransform t;
  for (int i=imgGroup.childItems().size(); i--; ) {
    QGraphicsItem* e=imgGroup.childItems().at(i);
    double x = e->pos().x();
    double y = e->pos().y();

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
    e->setPos(x,y);
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
  for (int n=0; n<markerNumber(); n++) {
    QPointF p=getMarkerDetPos(n);
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
          addMarker(QPointF(x,y));
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
    w->start(QThread::HighPriority);
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

void Projector::SpotMarkerGraphicsItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w) {  

  if (cache.size()!=p->viewport().size()) {
    cache = QPixmap(p->viewport().size());
    cacheNeedsUpdate = true;
  }

  if (cacheNeedsUpdate) {
    transform = p->worldTransform();
    workN = 0;
    workerStart.wakeAll();
    cache.fill(QColor(0,0,0,0));
    workerSync.acquire(workers.size());
    QPainter p2(&cache);
    for (int i=0; i<workers.size(); i++) {
      p2.drawImage(QPoint(0,0), workers.at(i)->localCache);
    }
  }

  p->save();
  p->resetTransform();
  p->drawPixmap(QPoint(0,0), cache);
  p->restore();


}


QRectF Projector::SpotMarkerGraphicsItem::boundingRect() const {
  return QRectF(-1,-1,2,2);
}


void Projector::SpotMarkerGraphicsItem::Worker::run() {
  forever {
    spotMarker->mutex.lock();
    spotMarker->workerStart.wait(&spotMarker->mutex);
    spotMarker->mutex.unlock();

    if (shouldStop) {
      spotMarker->workerSync.release(1);
      return;
    }

    double rx = spotMarker->transform.m11()*spotMarker->spotSize;
    double ry = spotMarker->transform.m22()*spotMarker->spotSize;

    if (localCache.size()!=spotMarker->cache.size())
      localCache = QImage(spotMarker->cache.size(), QImage::Format_ARGB32_Premultiplied);
    localCache.fill(QColor(0,0,0,0).rgba());
    QPainter p(&localCache);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::green);
    int i;
    while ((i=spotMarker->workN.fetchAndAddOrdered(1))<spotMarker->paintUntil) {
      p.drawEllipse(spotMarker->transform.map(spotMarker->coordinates.at(i)), rx, ry);
    }
    p.end();

    spotMarker->workerSync.release(1);

  }

}
