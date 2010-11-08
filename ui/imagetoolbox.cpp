#include "imagetoolbox.h"
#include "ui_imagetoolbox.h"

#include <QPen>
#include <QGraphicsPathItem>
#include <QMouseEvent>
#include <image/laueimage.h>
#include <image/BezierCurve.h>

ImageToolbox::ImageToolbox(LaueImage* img, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ImageToolbox),
    image(img)
{
  ui->setupUi(this);
  scene.setSceneRect(0,0,1,1);
  ui->gv->setScene(&scene);
  //ui->gv->scale(1, -1);

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
    activeCurve=-1;
    changeToCurve(n);
    updateCurveLines(n);
  }
  changeToCurve(0);
  connect(ui->ColorSelector, SIGNAL(activated(int)), this, SLOT(changeToCurve(int)));
}

ImageToolbox::~ImageToolbox()
{
  delete ui;
}

void ImageToolbox::newMarker(int curveId, const QPointF& p) {
  ImageToolbox::BoundedEllipse* item = new ImageToolbox::BoundedEllipse();
  item->setPosNoSig(p);
  scene.addItem(item);
  connect(item, SIGNAL(positionChanged()), this, SLOT(markerChanged()));
  handleMarkers << item;
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
      newMarker(0, p);
    }
  }
}

void ImageToolbox::updateCurveLines(int n) {
  QList<QPointF> pathPoints = bezierCurves[n]->pointRange(0.0, 0.01, 101);
  QPainterPath path(pathPoints[0]);
  for (int i=1; i<pathPoints.size(); i++) {
    path.lineTo(pathPoints[i]);
  }
  curves[activeCurve]->setPath(path);
}

void ImageToolbox::markerChanged() {
  QList<QPointF> points;
  foreach (QGraphicsItem* item, handleMarkers)
    points << item->pos();
  bezierCurves[activeCurve]->setPoints(points);
  updateCurveLines(activeCurve);
}

void ImageToolbox::resizeEvent(QResizeEvent *) {
  ui->gv->fitInView(0,0,1,1);
}

void ImageToolbox::mousePressEvent(QMouseEvent *e) {
  if (e->button()==Qt::LeftButton) {
    QPointF p = ui->gv->mapToScene(ui->gv->viewport()->mapFromGlobal(e->globalPos()));
    newMarker(0, p);
    updateCurveLines(activeCurve);
    //self.makeScales()
    //QMouseEvent me(e->type(), ui->gv->mapFromScene(p), e->globalPos(), e->button(), e->buttons(), e->modifiers());
    //ui->gv->mouseMoveEvent(&me);
/*      elif e.button()==QtCore.Qt.RightButton:
          idx=self.ColorSelector.currentIndex()
          for m in self.transferCurveMarkers[idx][1:-1]:
              if m.isUnderMouse():
                  self.transferCurveMarkers[idx].remove(m)
                  self.gs.removeItem(m)
                  self.updateTransferCurve(idx)
                  self.makeScales()
                  self.publishCurves()
*/
  }
}

ImageToolbox::BoundedEllipse::BoundedEllipse(QGraphicsItem *parent): SignalingEllipseItem(parent) {
  setRect(-0.02, -0.02, 0.04, 0.04);
  setFlag(QGraphicsItem::ItemIsMovable);
  setCursor(QCursor(Qt::SizeAllCursor));
}

QVariant ImageToolbox::BoundedEllipse::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change==ItemPositionChange) {
    QPointF p=value.toPointF();
    if (p.x()>1.0) {
      p.setX(1.0);
    } else if (p.x()<0.0) {
      p.setX(0.0);
    }
    if (p.y()>1.0) {
      p.setY(1.0);
    } else if (p.y()<0.0) {
      p.setY(0.0);
    }
    return SignalingEllipseItem::itemChange(change, QVariant(p));
  }
  return SignalingEllipseItem::itemChange(change, value);
}
