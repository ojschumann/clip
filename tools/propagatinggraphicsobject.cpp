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
