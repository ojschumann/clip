#include "contrastcurves.h"
#include "ui_contrastcurves.h"

#include <QFile>
#include <QtXml/QDomDocument>
#include <QTextStream>
#include <QMouseEvent>

#include "image/BezierCurve.h"
#include "core/projector.h"
#include "image/laueimage.h"


ContrastCurves::ContrastCurves(Projector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContrastCurves),
    projector(p),
    handleMarkers(),
    curves(),
    histogram()
{
    ui->setupUi(this);
    scene.setSceneRect(0,0,1,1);
    ui->gv->setScene(&scene);
    ui->gv->scale(1, -1);

    QPen decoPen(Qt::gray, 0, Qt::DotLine);
    QList<double> l;
    l << 0.0 << 0.25 << 0.5 << 0.75 << 1.0;
    foreach (double d, l) {
      scene.addLine(0, d, 1, d);
      scene.addLine(d, 0, d, 1);
    }

    QList<QColor> cl;
    cl << Qt::black << Qt::red << Qt::green << Qt::blue;
    for (int n=0; n<cl.size(); n++) {
      curves << scene.addPath(QPainterPath(), QPen(cl[n], 0));
      curves.last()->setZValue(4.0-n);
      updateCurveLines(n);
    }
    activeCurve = ui->ColorSelector->currentIndex()+1;
    changeToCurve(ui->ColorSelector->currentIndex());
    connect(ui->ColorSelector, SIGNAL(activated(int)), this, SLOT(changeToCurve(int)));
    connect(projector->getLaueImage()->getScaler(), SIGNAL(histogramChanged(QVector<int>,QVector<int>,QVector<int>)), this, SLOT(updateHistogram(QVector<int>,QVector<int>,QVector<int>)));
  }

ContrastCurves::~ContrastCurves()
{
    delete ui;
}

void ContrastCurves::changeToCurve(int n) {
  if (projector.isNull() || projector->getLaueImage()==0) return;
  activeCurve=n;
  foreach (BoundedEllipse* item, handleMarkers) {
    scene.removeItem(item);
    delete item;
  }
  handleMarkers.clear();
  BezierCurve* curve = projector->getLaueImage()->getTransferCurves()[n];
  foreach (QPointF p, curve->getPoints()) {
    newMarker(p);
  }
  handleMarkers.first()->setBBox(QRectF(0,0,0,1));
  handleMarkers.first()->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
  handleMarkers.last()->setBBox(QRectF(1,0,0,1));
  handleMarkers.last()->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);

}

void ContrastCurves::newMarker(const QPointF& p) {
  ContrastCurves::BoundedEllipse* item = new ContrastCurves::BoundedEllipse();
  item->setPosNoSig(p);
  scene.addItem(item);
  item->setBBox(scene.sceneRect());
  item->setZValue(10);
  connect(item, SIGNAL(positionChanged()), this, SLOT(markerChanged()));
  handleMarkers << item;
  markerChanged();
}

void ContrastCurves::updateCurveLines(int n) {
  if (projector.isNull() || projector->getLaueImage()==0) return;
  BezierCurve* curve = projector->getLaueImage()->getTransferCurves()[n];
  QList<QPointF> pathPoints = curve->pointRange(0.0, 0.01, 101);
  QPainterPath path(pathPoints[0]);
  for (int i=1; i<pathPoints.size(); i++) {
    path.lineTo(pathPoints[i]);
  }
  curves[n]->setPath(path);
}

void ContrastCurves::markerChanged() {
  if (handleMarkers.size()<2)
    return;
  QMap<double, QPointF> points;
  foreach (QGraphicsItem* item, handleMarkers)
    points.insert(item->x(), item->pos());

  if (projector.isNull() || projector->getLaueImage()==0) return;
  BezierCurve* curve = projector->getLaueImage()->getTransferCurves()[activeCurve];
  curve->setPoints(points.values());
  updateCurveLines(activeCurve);
  makeScales();
}

void ContrastCurves::mousePressEvent(QMouseEvent *e) {
  QPointF p = ui->gv->mapToScene(ui->gv->viewport()->mapFromGlobal(e->globalPos()));
  if (e->button()==Qt::LeftButton) {
    newMarker(p);
    QMouseEvent me(e->type(), ui->gv->mapFromScene(p), e->globalPos(), e->button(), e->buttons(), e->modifiers());
    ui->gv->mousePressEvent(&me);
  } else if (e->button()==Qt::RightButton) {
    BoundedEllipse* item = dynamic_cast<BoundedEllipse*>(scene.itemAt(p, ui->gv->transform()));
    if (item) {
      scene.removeItem(item);
      delete item;
      handleMarkers.removeAll(item);
      markerChanged();
    }
  }
}

void ContrastCurves::makeScales() {
  makeVScale();
  makeHScale();
}

void ContrastCurves::makeVScale() {
  QPixmap vScalePix(ui->verticalScale->size());
  QPainter p(&vScalePix);
  int w = vScalePix.width();
  int h = vScalePix.height();
  if (projector.isNull() || projector->getLaueImage()==0) return;
  QList<BezierCurve*> bezierCurves = projector->getLaueImage()->getTransferCurves();
  QList<float> V = bezierCurves[0]->range(0.0, 1.0/(h-1), h);
  QList<float> R = bezierCurves[1]->map(V);
  QList<float> G = bezierCurves[2]->map(V);
  QList<float> B = bezierCurves[3]->map(V);

  for (int y=0; y<h; y++) {
    int v = int(255.0*V[y]);
    int r = int(255.0*R[y]);
    int g = int(255.0*G[y]);
    int b = int(255.0*B[y]);
    p.setPen(QColor(r,g,b));
    p.drawLine(0, h-y-1, w/2, h-y-1);

    if (ui->ColorSelector->currentIndex()==0) {
      p.setPen(QColor(v,v,v));
    } else if (ui->ColorSelector->currentIndex()==1) {
      p.setPen(QColor(r, 0, 0));
    } else if (ui->ColorSelector->currentIndex()==2) {
      p.setPen(QColor(0, g, 0));
    } else if (ui->ColorSelector->currentIndex()==3) {
      p.setPen(QColor(0, 0, b));
    }
    p.drawLine(w/2, h-y-1, w, h-y-1);

  }
  p.end();
  ui->verticalScale->setPixmap(vScalePix);

}

void ContrastCurves::makeHScale() {
  QPixmap hScalePix(ui->horizontalScale->size());
  QPainter p(&hScalePix);
  int w = hScalePix.width();
  int h = hScalePix.height();
  for (int x=0; x<w; x++) {
    int col = int(255.0*x/(w-1));
    p.setPen(QColor(col, col, col));
    p.drawLine(x, 0, x, h);
  }
  p.end();
  ui->horizontalScale->setPixmap(hScalePix);
}



ContrastCurves::BoundedEllipse::BoundedEllipse(QGraphicsItem *parent): SignalingEllipseItem(parent) {
  setRect(-5,-5,10,10);
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIgnoresTransformations);
  setAcceptedMouseButtons(Qt::LeftButton);
  setCursor(QCursor(Qt::SizeAllCursor));
}


bool ContrastCurves::BoundedEllipse::operator<(const BoundedEllipse& o) {
  return x()<o.x();
}

QVariant ContrastCurves::BoundedEllipse::itemChange(GraphicsItemChange change, const QVariant &value) {
  if ((change==ItemPositionChange) && !bbox.isNull()) {
    QPointF p=value.toPointF();
    if (p.x()>bbox.right()) {
      p.setX(bbox.right());
    } else if (p.x()<bbox.left()) {
      p.setX(bbox.left());
    }
    if (p.y()<bbox.top()) {
      p.setY(bbox.top());
    } else if (p.y()>bbox.bottom()) {
      p.setY(bbox.bottom());
    }
    return SignalingEllipseItem::itemChange(change, QVariant(p));
  }
  return SignalingEllipseItem::itemChange(change, value);
}


void ContrastCurves::loadFromFile(const QString& filename) {
  QDomDocument doc("curves");
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
      return;
  if (!doc.setContent(&file)) {
      file.close();
      return;
  }
  file.close();

  QStringList curveNames;
  curveNames << "Value" << "Red" << "Green" << "Blue";
  QList< QList<QPointF> > allCurvePoints;
  foreach (QString name, curveNames) {
    QDomElement curve = doc.elementsByTagName(name).at(0).toElement();
    if (curve.isNull()) return;
    QList<QPointF> points;
    QDomNodeList curvePointElements = curve.elementsByTagName("Point");
    for (int i=0; i<curvePointElements.size(); i++) {
      bool okx, oky;
      double x = curvePointElements.at(i).toElement().attribute("x").toDouble(&okx);
      double y = curvePointElements.at(i).toElement().attribute("y").toDouble(&oky);
      if (!okx || !oky || x<0.0 || x>1.0 || y<0.0 || y>1.0) return;
      points << QPointF(x,y);
    }
    if (points.size()<2) return;
    allCurvePoints << points;
  }
  if (projector.isNull() || projector->getLaueImage()==0) return;
  QList<BezierCurve*> bezierCurves = projector->getLaueImage()->getTransferCurves();
  for (int i=0; i<4; i++) {
    bezierCurves[i]->setPoints(allCurvePoints[i]);
    updateCurveLines(i);
  }
  changeToCurve(activeCurve);
}



void ContrastCurves::saveToFile(const QString& filename) {
  if (projector.isNull() || projector->getLaueImage()==0) return;
  QList<BezierCurve*> bezierCurves = projector->getLaueImage()->getTransferCurves();

  QDomDocument doc("curves");
  QDomNode base = doc.appendChild(doc.createElement("Transfercurves"));
  QStringList curveNames;
  curveNames << "Value" << "Red" << "Green" << "Blue";
  QList<BezierCurve*>::Iterator it = bezierCurves.begin();
  foreach (QString name, curveNames) {
    QDomNode curve = base.appendChild(doc.createElement(name));
    foreach (QPointF p, (*it)->getPoints()) {
      QDomElement point = curve.appendChild(doc.createElement("Point")).toElement();
      point.setAttribute("x", p.x());
      point.setAttribute("y", p.y());
    }
    it++;
  }
  QFile file(filename);

  if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
    QTextStream ts(&file);
    doc.save(ts, 0);
    file.close();
  }
}

void ContrastCurves::updateHistogram(QVector<int> r, QVector<int> g, QVector<int> b) {
  if (histogram.isEmpty()) {
    QList<QColor> colors;
    colors << QColor(0xFF, 0, 0, 0x7F)  << QColor(0, 0xFF, 0, 0x7F)  << QColor(0, 0, 0xFF, 0x7F);
    foreach (QColor color, colors) {
      QGraphicsPathItem* h = new QGraphicsPathItem();
      h->setPen(Qt::NoPen);
      h->setBrush(QBrush(color));
      scene.addItem(h);
      histogram << h;
    }
  }
  QList< QPair< QVector<int>, QGraphicsPathItem*> > l;
  l << qMakePair(r, histogram[0])  << qMakePair(g, histogram[1]) << qMakePair(b, histogram[2]);
  QPair< QVector<int>, QGraphicsPathItem*> pair;
  foreach (pair, l) {
    QPainterPath path;
    path.moveTo(0,0);
    double maxValue=0;
    for (int i=0; i<pair.first.size(); i++) {
      double val = pair.first[i];
      if (val>0) val = log(val);
      path.lineTo(1.0*i/255, val);
      if (maxValue<val) maxValue = val;
    }
    path.lineTo(1,0);
    path.lineTo(0,0);
    pair.second->setPath(path);
    pair.second->setTransform(QTransform::fromScale(1, 1.0/maxValue));
  }
}
