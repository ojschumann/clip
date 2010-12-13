#include "cropmarker.h"
#include <QPainter>
#include <QPen>
#include <QCursor>
#include <cmath>
#include <QGraphicsSceneMouseEvent>


#include "tools/circleitem.h"

CropMarker::CropMarker(const QPointF &pCenter, double _dx, double _dy, double _angle, double _handleSize, QGraphicsItem *parent):
    QGraphicsObject(parent)
{
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setFlag(QGraphicsItem::ItemIsMovable);
  setCursor(QCursor(Qt::OpenHandCursor));
  setHandleSize(handleSize);

  angle = 10.0;
  size.setWidth(20*_handleSize);
  size.setHeight(20*_handleSize);

  setRotation(angle);
  handleSize = _handleSize;

  QPen pen(Qt::green);

  for (int i=0; i<9; i++) {
    QGraphicsRectItem* handle = new QGraphicsRectItem(this);
    handle->setPen(pen);
    handles << handle;
  }

  handles.at(0)->setCursor(QCursor(Qt::SizeVerCursor));
  handles.at(1)->setCursor(QCursor(Qt::SizeVerCursor));
  handles.at(2)->setCursor(QCursor(Qt::SizeHorCursor));
  handles.at(3)->setCursor(QCursor(Qt::SizeHorCursor));
  handles.at(4)->setCursor(QCursor(Qt::SizeAllCursor));
  handles.at(5)->setCursor(QCursor(Qt::SizeAllCursor));
  handles.at(6)->setCursor(QCursor(Qt::SizeAllCursor));
  handles.at(7)->setCursor(QCursor(Qt::SizeAllCursor));
  handles.at(8)->setCursor(QCursor(Qt::ClosedHandCursor));

  positionHandles();
}

void CropMarker::positionHandles() {
  double w = size.width()/2;
  double h = size.height()/2;
  double d = handleSize;

  handles.at(0)->setRect(-w+d, h-d, (w-d)*2, d);
  handles.at(1)->setRect(-w+d, -h, (w-d)*2, d);
  handles.at(2)->setRect(-w, -h+d, d, 2*(h-d));
  handles.at(3)->setRect(w-d, -h+d, d, 2*(h-d));

  handles.at(4)->setRect(-w  ,   -h, d, d);
  handles.at(5)->setRect( w-d,   -h, d, d);
  handles.at(6)->setRect(-w  ,  h-d, d, d);
  handles.at(7)->setRect( w-d,  h-d, d, d);

  handles.at(8)->setRect(-d, h-2*d, 2*d, 2*d);
}

QPolygonF CropMarker::getRect() {
  QPolygonF rect;

  return rect;
}

void CropMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

  QPen pen = painter->pen();
  pen.setStyle(Qt::DashLine);
  pen.setColor(Qt::red);
  painter->setPen(pen);


  QPolygonF p;
  p << QPointF( -size.width()/2, -size.height()/2);
  p << QPointF(  size.width()/2, -size.height()/2);
  p << QPointF(  size.width()/2,  size.height()/2);
  p << QPointF( -size.width()/2,  size.height()/2);
  painter->drawPolygon(p);

  p.clear();
  p << QPointF( -handleSize, size.height()/2);
  p << QPointF( -handleSize, size.height()/2-2*handleSize);
  p << QPointF(  handleSize, size.height()/2-2*handleSize);
  p << QPointF(  handleSize, size.height()/2);

  painter->drawPolyline(p);


}

QRectF CropMarker::boundingRect() const {
  return QRectF (-size.width()/2, -size.height()/2, size.width(), size.height());
}


void CropMarker::setHandleSize(double s) {
  handleSize = s;
  update();
}


void CropMarker::setImgTransform(const QTransform &t) {
  foreach (QGraphicsItem* item, childItems()) {
    item->setTransform(t, true);
    item->setPos(t.map(item->pos()));
  }
}
#include "debug.h"


void CropMarker::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  pressedOnHandle=-1;

  if (e->buttons()==Qt::LeftButton) {
    for (int i=0; i<handles.size(); i++) {
      if (handles.at(i)->contains(e->pos())) {
        pressedOnHandle = i;
        e->accept();
        return;
      }
    }
  }
  QGraphicsObject::mousePressEvent(e);
}

void CropMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  if (pressedOnHandle==0) {
    double dy = e->pos().y()-e->lastPos().y();
    size.rheight()+=dy;
    prepareGeometryChange();
    positionHandles();
    double a = M_PI/180.0*rotation();
    moveBy(-0.5*dy*sin(a), 0.5*dy*cos(a));
  } else if (pressedOnHandle==1) {
    double dy = e->pos().y()-e->lastPos().y();
    size.rheight()-=dy;
    prepareGeometryChange();
    positionHandles();
    double a = M_PI/180.0*rotation();
    moveBy(-0.5*dy*sin(a), 0.5*dy*cos(a));
  } else if (pressedOnHandle==2) {
    double dx = e->pos().x()-e->lastPos().x();
    size.rwidth()-=dx;
    prepareGeometryChange();
    positionHandles();
    double a = M_PI/180.0*rotation();
    moveBy(0.5*dx*cos(a), 0.5*dx*sin(a));
  } else if (pressedOnHandle==8) {
    double lastAngle = atan2(e->lastPos().y(), e->lastPos().x());
    double thisAngle = atan2(e->pos().y(), e->pos().x());
    cout << 180.0*M_1_PI*(thisAngle-lastAngle) << endl;
    setRotation(rotation()+180.0*M_1_PI*(thisAngle-lastAngle));
  } else {
    QGraphicsObject::mouseMoveEvent(e);
  }
}
