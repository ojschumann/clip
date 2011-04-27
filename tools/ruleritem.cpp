/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

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

#include "ruleritem.h"
#include <QPainter>
#include <QCursor>
#include <iostream>
#include "tools/circleitem.h"
#include "config/configstore.h"

using namespace std;


RulerItem::RulerItem(const QPointF& p1, const QPointF& p2, double r, QGraphicsItem* parent):
    PropagatingGraphicsObject(parent),
    startHandle(new CircleItem(r, this)),
    endHandle(new CircleItem(r,this)),
    highlighted(true),
    radius(r)
{
  highlight(false);
  startHandle->setPos(p1);
  endHandle->setPos(p2);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  QList<CircleItem*> l = QList<CircleItem*>() << startHandle << endHandle;
  foreach (CircleItem* item, l) {
    ConfigStore::getInstance()->ensureColor(ConfigStore::RulerHandles, item, SLOT(setColor(QColor)));
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setCursor(QCursor(Qt::SizeAllCursor));
    connect(item, SIGNAL(itemClicked()),     this, SIGNAL(itemClicked()));
    connect(item, SIGNAL(positionChanged()), this, SIGNAL(positionChanged()));
  }
  ConfigStore::getInstance()->ensureColor(ConfigStore::Ruler, this, SLOT(setColor(QColor)));
}

RulerItem::~RulerItem() {

}

void RulerItem::setColor(const QColor &c) {
  rulerColor = c;
  update();
}

QRectF RulerItem::boundingRect() const {
  return childrenBoundingRect();
}

QPainterPath RulerItem::shape() const {
  QPainterPath path(startHandle->pos());
  path.addEllipse(startHandle->pos(), radius, radius);
  path.moveTo(endHandle->pos());
  path.addEllipse(endHandle->pos(), radius, radius);
  return path;
}

void RulerItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) {
  if (startHandle->pos()!=endHandle->pos()) {
    QPen pen(ConfigStore::getInstance()->color(ConfigStore::Ruler));
    if (highlighted) {
      pen.setWidthF(2.5);
    } else {
      pen.setWidthF(0);
    }
    pen.setCosmetic(true);
    p->setPen(pen);
    QVector<QLineF> lines;
    QLineF l(startHandle->pos(), endHandle->pos());
    p->drawLine(l);

    l.setLength(0.7*radius);
    l.setAngle(l.angle()+30.0);
    lines << l;
    l.setAngle(l.angle()-60.0);
    lines << l;
    l = QLineF(endHandle->pos(), startHandle->pos());
    l.setLength(0.7*radius);
    l.setAngle(l.angle()+30.0);
    lines << l;
    l.setAngle(l.angle()-60.0);
    lines << l;

    pen.setWidth(0);
    p->setPen(pen);
    p->drawLines(lines);
  }
}

void RulerItem::setStart(const QPointF& p) {
  startHandle->setPos(p);
}

void RulerItem::setEnd(const QPointF& p) {
  endHandle->setPos(p);
}

QPointF RulerItem::getStart() {
  return startHandle->pos();
}

QPointF RulerItem::getEnd() {
  return endHandle->pos();
}

void RulerItem::highlight(bool h) {
  highlighted=h;
  update();
}

bool RulerItem::isHighlighted() {
  return highlighted;
}

void RulerItem::setHandleSize(double r) {
  radius = r;
  startHandle->setRadius(radius);
  endHandle->setRadius(radius);
}
