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

#include "spotitem.h"

#include <QPainter>

#include "core/projector.h"
#include "config/configstore.h"

SpotItem::SpotItem(Projector *p, double r, QGraphicsItem *parent):
  CircleItem(r, parent),
  AbstractProjectorMarkerItem(p, AbstractMarkerItem::SpotMarker)
{
  highlight(false);
  connect(this, SIGNAL(positionChanged()), this, SLOT(slotInvalidateCache()));

  setFlag(QGraphicsItem::ItemIsMovable, true);
  setCursor(QCursor(Qt::SizeAllCursor));
  ConfigStore::getInstance()->ensureColor(ConfigStore::SpotMarker, this, SLOT(setColor(QColor)));
  ConfigStore::getInstance()->ensureColor(ConfigStore::SpotMarkerHighlight, this, SLOT(doUpdate()));
  ConfigStore::getInstance()->ensureColor(ConfigStore::SpotMarkerHighlightBg, this, SLOT(doUpdate()));
}


Vec3D SpotItem::getMarkerNormal() const {
  return projector->det2normal(projector->img2det.map(pos()));
}

void SpotItem::highlight(bool b) {
  //setLineWidth(b?2.0:1.0);
  if (b!=isHighlighted) {
    setZValue(b ? 10 : 0);
    isHighlighted = b;
    prepareGeometryChange();
    update();
  }
}

void SpotItem::slotInvalidateCache() {
  invalidateCache();
}

void SpotItem::slotSetMaxSearchIndex(int n) {
  setMaxSearchIndex(n);
}

void SpotItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  if (isHighlighted) {
    ConfigStore* config = ConfigStore::getInstance();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(config->color(ConfigStore::SpotMarkerHighlightBg)));
    painter->drawEllipse(QPointF(0,0), 2*radius, 2*radius);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(config->color(ConfigStore::SpotMarkerHighlight));
    painter->drawEllipse(QPointF(0,0), radius, radius);
  } else {
    CircleItem::paint(painter, option, widget);
  }
}

QRectF SpotItem::boundingRect() const {
  if (isHighlighted) {
    return QRectF(-2*radius, -2*radius, 4*radius, 4*radius);
  } else {
    return CircleItem::boundingRect();
  }
}

QPainterPath SpotItem::shape() const {
  if (isHighlighted) {
    QPainterPath path;
    path.addEllipse(QPointF(0,0), 2*radius, 2*radius);
    return path;
  } else {
    return CircleItem::shape();
  }
}
