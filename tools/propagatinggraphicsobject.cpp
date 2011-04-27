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

#include "propagatinggraphicsobject.h"

#include <iostream>
#include <iomanip>

using namespace std;


PropagatingGraphicsObject::PropagatingGraphicsObject(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
  setFlag(ItemSendsGeometryChanges);
  propagatePositionChange = true;
  propagateTransformChange = true;

}

QVariant PropagatingGraphicsObject::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (propagateTransformChange  && (change == ItemTransformChange)) {
    propagatedTransform = value.value<QTransform>();
    foreach (QGraphicsItem* item, childItems())
      item->setTransform(propagatedTransform);
    return QVariant(transform());
  } else if (propagatePositionChange  && (change == ItemPositionChange)) {
    QPointF dp = value.value<QPointF>()-pos();
    foreach (QGraphicsItem* item, childItems())
      item->setPos(item->pos()+dp);
    return QVariant(pos());
#if QT_VERSION >= 0x040700 // Rotation and scaling reported seperately
  } else if (propagateTransformChange  && (change == ItemRotationChange)) {
    double newRotation = value.toDouble();
    foreach (QGraphicsItem* item, childItems())
      item->setRotation(newRotation);
    return QVariant(rotation());
  } else if (propagateTransformChange  && (change == ItemScaleChange)) {
    double newScale = value.toDouble();
    foreach (QGraphicsItem* item, childItems())
      item->setScale(newScale);
    return QVariant(scale());
#endif
  }
  return QGraphicsItem::itemChange(change, value);
}

void PropagatingGraphicsObject::setImgTransform(const QTransform &t) {
  foreach (QGraphicsItem* subitem, childItems())
    subitem->setPos(t.map(subitem->pos()));
}
