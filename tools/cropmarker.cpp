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

#include "cropmarker.h"
#include <QPainter>
#include <QPen>
#include <QCursor>
#include <cmath>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QTimer>

#include "config/configstore.h"


CropMarker::CropMarker(const QPointF& /*pCenter*/, double /*_dx*/, double /*_dy*/, double /*_angle*/, double _handleSize, QGraphicsItem *_parent):
    QGraphicsObject(_parent)
{
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setFlag(QGraphicsItem::ItemIsMovable);
  setCursor(QCursor(Qt::OpenHandCursor));
  setHandleSize(handleSize);

  size.setWidth(20*_handleSize);
  size.setHeight(20*_handleSize);

  setRotation(0);
  handleSize = _handleSize;

  QPen pen(Qt::NoPen);
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
  handles.at(8)->setCursor(QCursor(QPixmap(":/cursor_rot.png")));

  positionHandles();
  setCursors();

  ConfigStore::getInstance()->ensureColor(ConfigStore::CropMarkerOutline, this, SLOT(setOutlineColor(QColor)));
  ConfigStore::getInstance()->ensureColor(ConfigStore::CropMarkerInterior, this, SLOT(setInteriorColor(QColor)));
}

CropMarker::~CropMarker() {

}

void CropMarker::positionHandles() {
  double w = size.width()/2;
  double h = size.height()/2;
  double d = 2.0*handleSize;

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

void CropMarker::setCursors() {

  QPixmap cursorPixmap(":/cursor_hor.png");

  QTransform t;
  t.rotate(-rotation());
  cursorPixmap = cursorPixmap.transformed(t, Qt::SmoothTransformation);

  QCursor newCursor(cursorPixmap);
  handles.at(2)->setCursor(newCursor);
  handles.at(3)->setCursor(newCursor);

  t.reset();
  t.rotate(90);
  newCursor=QCursor(cursorPixmap.transformed(t));
  handles.at(0)->setCursor(newCursor);
  handles.at(1)->setCursor(newCursor);
}

QPolygonF CropMarker::getRect() {
  QPolygonF rect;

  return rect;
}

void CropMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
  double w = size.width()/2;
  double h = size.height()/2;
  double d = 2.0*handleSize;

  QPen pen = painter->pen();
  pen.setStyle(Qt::DashLine);
  pen.setColor(outlineColor);
  painter->setPen(pen);

  QPolygonF p;
  p << QPointF( -w, -h);
  p << QPointF(  w, -h);
  p << QPointF(  w,  h);
  p << QPointF( -w,  h);
  painter->drawPolygon(p);

  p.clear();
  p << handles.at(8)->rect().bottomLeft();
  p << handles.at(8)->rect().topLeft();
  p << handles.at(8)->rect().topRight();
  p << handles.at(8)->rect().bottomRight();
  painter->drawPolyline(p);

  pen.setColor(interiorColor);
  painter->setPen(pen);
  painter->drawLine(QPointF( -w, h-d), QPointF(  w,  h-d));
  painter->drawLine(QPointF( -w, d-h), QPointF(  w,  d-h));
  painter->drawLine(QPointF(d-w,  -h), QPointF(d-w,    h));
  painter->drawLine(QPointF(w-d,  -h), QPointF(w-d,    h));
}

QRectF CropMarker::boundingRect() const {
  return QRectF (-size.width()/2, -size.height()/2, size.width(), size.height()).normalized();
}

QPainterPath CropMarker::shape() const {
  QPainterPath path;
  path.addRect(boundingRect());
  return path;
}


void CropMarker::setHandleSize(double s) {
  handleSize = s;
  update();
}

void CropMarker::setOutlineColor(const QColor& c) {
  outlineColor = c;
  update();
}

void CropMarker::setInteriorColor(const QColor& c) {
  interiorColor = c;
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
    setCursor(QCursor(Qt::ClosedHandCursor));
    // From qgraphicsscene.cpp
    QGraphicsView* view = e->widget() ? qobject_cast<QGraphicsView *>(e->widget()->parentWidget()) : 0;
    QList<QGraphicsItem*> items;
    if (!view) {
      items = scene()->items(e->scenePos(), Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());
    } else {
      const QRectF pointRect(QPointF(e->widget()->mapFromGlobal(e->screenPos())), QSizeF(1, 1));
      const QTransform viewTransform = view->viewportTransform();
      items = scene()->items(viewTransform.inverted().map(pointRect), Qt::IntersectsItemShape,
                             Qt::DescendingOrder, viewTransform);
    }
    for (int i=handles.size(); i--; ) {
      if (items.contains(handles.at(i))) {
        pressedOnHandle = i;
        e->accept();
        return;
      }
    }
  } else if (e->buttons()==Qt::RightButton) {
    //emit cancelCrop();
    QTimer::singleShot(0, this, SLOT(deleteLater()));
  }
  QGraphicsObject::mousePressEvent(e);
}

void CropMarker::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  setCursor(QCursor(Qt::OpenHandCursor));
  QGraphicsObject::mouseReleaseEvent(e);
}


void CropMarker::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) {
  doPublishCrop();
  e->accept();
}

void CropMarker::doPublishCrop() {
  QPolygonF rect;
  double w = size.width()/2;
  double h = size.height()/2;
  rect << QPointF(-w,-h);
  rect << QPointF( w,-h);
  rect << QPointF( w, h);
  rect << QPointF(-w, h);
  emit publishCrop(mapToParent(rect));
}

void CropMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  if ((pressedOnHandle>=0) && (pressedOnHandle<8)) {
    double dx = e->pos().x()-e->lastPos().x();
    double dy = e->pos().y()-e->lastPos().y();
    double mx = 0.0;
    double my = 0.0;
    if (pressedOnHandle==0) {
      my = 1.0;
    } else if (pressedOnHandle==1) {
      my = -1.0;
    } else if (pressedOnHandle==2) {
      mx = -1.0;
    } else if (pressedOnHandle==3) {
      mx = 1.0;
    } else if (pressedOnHandle==4) {
      mx = -1.0;
      my = -1.0;
    } else if (pressedOnHandle==5) {
      mx = 1.0;
      my = -1.0;
    } else if (pressedOnHandle==6) {
      mx = -1.0;
      my = 1.0;
    } else if (pressedOnHandle==7) {
      mx = 1.0;
      my = 1.0;
    }
    dx = std::max(dx*mx, 5*handleSize-size.width());
    dy = std::max(dy*my, 5*handleSize-size.height());
    size.rwidth() += dx;
    size.rheight() += dy;
    prepareGeometryChange();
    positionHandles();
    double a = M_PI/180.0*rotation();
    moveBy(0.5*(mx*dx*cos(a)-my*dy*sin(a)), 0.5*(my*dy*cos(a)+mx*dx*sin(a)));
  } else if (pressedOnHandle==8) {
    double lastAngle = atan2(e->lastPos().y(), e->lastPos().x());
    double thisAngle = atan2(e->pos().y(), e->pos().x());
    setRotation(rotation()+180.0*M_1_PI*(thisAngle-lastAngle));
    setCursors();
  } else {
    QGraphicsObject::mouseMoveEvent(e);
  }
}
