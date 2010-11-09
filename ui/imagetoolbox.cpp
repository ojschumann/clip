#include "imagetoolbox.h"
#include "ui_imagetoolbox.h"

#include <QPen>
#include <QGraphicsPathItem>
#include <QMouseEvent>
#include <image/laueimage.h>
#include <image/BezierCurve.h>

ImageToolbox::ImageToolbox(LaueImage* img, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::ImageToolbox),
    image(img),
    bezierCurves(),
    handleMarkers(),
    curves()
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
    bezierCurves << new BezierCurve();
    updateCurveLines(n);
  }
  activeCurve = ui->ColorSelector->currentIndex()+1;
  changeToCurve(ui->ColorSelector->currentIndex());
  connect(ui->ColorSelector, SIGNAL(activated(int)), this, SLOT(changeToCurve(int)));
}

ImageToolbox::~ImageToolbox()
{
  delete ui;
}

void ImageToolbox::changeToCurve(int n) {
  if (n!=activeCurve) {
    activeCurve=n;
    foreach (BoundedEllipse* item, handleMarkers) {
      scene.removeItem(item);
      delete item;
    }
    handleMarkers.clear();
    foreach (QPointF p, bezierCurves[n]->getPoints()) {
      newMarker(p);
    }
    handleMarkers.first()->setBBox(QRectF(0,0,0,1));
    handleMarkers.first()->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
    handleMarkers.last()->setBBox(QRectF(1,0,0,1));
    handleMarkers.last()->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
  }
}

void ImageToolbox::newMarker(const QPointF& p) {
  ImageToolbox::BoundedEllipse* item = new ImageToolbox::BoundedEllipse();
  item->setPosNoSig(p);
  scene.addItem(item);
  item->setBBox(scene.sceneRect());
  item->setZValue(10);
  connect(item, SIGNAL(positionChanged()), this, SLOT(markerChanged()));
  handleMarkers << item;
  markerChanged();
}

void ImageToolbox::updateCurveLines(int n) {
  QList<QPointF> pathPoints = bezierCurves[n]->pointRange(0.0, 0.01, 101);
  QPainterPath path(pathPoints[0]);
  for (int i=1; i<pathPoints.size(); i++) {
    path.lineTo(pathPoints[i]);
  }
  curves[n]->setPath(path);
}

void ImageToolbox::markerChanged() {
  if (handleMarkers.size()<2)
    return;
  QMap<double, QPointF> points;
  foreach (QGraphicsItem* item, handleMarkers)
    points.insert(item->x(), item->pos());

  bezierCurves[activeCurve]->setPoints(points.values());
  updateCurveLines(activeCurve);
  makeScales();
}

void ImageToolbox::mousePressEvent(QMouseEvent *e) {
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

void ImageToolbox::makeScales() {
  makeVScale();
  makeHScale();
}

void ImageToolbox::makeVScale() {
  QPixmap vScalePix(ui->verticalScale->size());
  QPainter p(&vScalePix);
  int w = vScalePix.width();
  int h = vScalePix.height();

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

void ImageToolbox::makeHScale() {
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



ImageToolbox::BoundedEllipse::BoundedEllipse(QGraphicsItem *parent): SignalingEllipseItem(parent) {
  setRect(-5,-5,10,10);
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIgnoresTransformations);
  setAcceptedMouseButtons(Qt::LeftButton);
  setCursor(QCursor(Qt::SizeAllCursor));
}


bool ImageToolbox::BoundedEllipse::operator<(const BoundedEllipse& o) {
  return x()<o.x();
}

QVariant ImageToolbox::BoundedEllipse::itemChange(GraphicsItemChange change, const QVariant &value) {
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
