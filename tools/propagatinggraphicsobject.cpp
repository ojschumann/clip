#include "propagatinggraphicsobject.h"

PropagatingGraphicsObject::PropagatingGraphicsObject(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
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
  }
  return QGraphicsItem::itemChange(change, value);
}

void PropagatingGraphicsObject::setImgTransform(const QTransform &t) {
  foreach (QGraphicsItem* subitem, childItems())
    subitem->setPos(t.map(subitem->pos()));
}
