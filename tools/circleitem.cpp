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

#include "circleitem.h"
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>
#include <QtDebug>
 

#include "tools/tools.h"

using namespace std;

CircleItem::CircleItem(double r, QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
  skipNextPosChange=false;
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setFlag(QGraphicsItem::ItemClipsToShape, false);
  setRadius(r);
  lineWidth = 1.0;
}

CircleItem::~CircleItem() {
}

void CircleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
  //PDF-Export via QPrinter::setOutputFormat(PdfFormat) has a Bug concerning
  //Cosmetic Pens and very small coordinates (here, the rect is (0, 0, 1, 1))
  //thus reset the World Transform and paint with noncosmetic pens

  QTransform t = painter->worldTransform();
  painter->resetTransform();
  QPen pen(color);
  pen.setWidthF(lineWidth);
  pen.setCosmetic(false);
  painter->setPen(pen);
  painter->setBrush(Qt::NoBrush);
  painter->drawEllipse(t.map(QPointF(0,0)), radius*t.m11(), radius*t.m22());
  painter->setTransform(t);
}

QRectF CircleItem::boundingRect() const {
  return QRectF(-radius, -radius, 2*radius, 2*radius);
}

QPainterPath CircleItem::shape() const {
  QPainterPath path;
  path.addEllipse(QPointF(0,0), radius, radius);
  return path;
}

bool CircleItem::contains(const QPointF &point) const {
  double r = fasthypot(point.x(), point.y());
  return r<radius;
}

QVariant CircleItem::itemChange(GraphicsItemChange change, const QVariant & value ) {
  if (change==ItemPositionHasChanged) {
    if (not skipNextPosChange) {
      emit positionChanged();
    } else {
      skipNextPosChange=false;
    }
  }
  return QGraphicsObject::itemChange(change, value);
}

void CircleItem::setPosNoSig(const QPointF &p) {
  if (p!=pos()) {
    skipNextPosChange=true;
    setPos(p);
  }
}

void CircleItem::setRadius(double r) {
  prepareGeometryChange();
  radius=r;
}

void CircleItem::setColor(QColor c) {
  color = c;
  update();
}

void CircleItem::setLineWidth(double lw) {
  lineWidth = lw;
  update();
}

void CircleItem::doUpdate() {
  update();
}


 
using namespace std;
void CircleItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (event->button()==Qt::LeftButton) emit itemClicked();
  QGraphicsObject::mousePressEvent(event);
}
