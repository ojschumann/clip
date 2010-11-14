#include "propagatinggraphicsobject.h"

PropagatingGraphicsObject::PropagatingGraphicsObject(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
}

QVariant PropagatingGraphicsObject::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == ItemTransformChange) {
    foreach (QGraphicsItem* item, childItems())
      item->setTransform(value.value<QTransform>());
    return QVariant(transform());
  } else if (change == ItemPositionChange) {
    QPointF dp = value.value<QPointF>()-pos();
    foreach (QGraphicsItem* item, childItems())
      item->setPos(item->pos()+dp);
    return QVariant(pos());
  }
  return QGraphicsItem::itemChange(change, value);
}
