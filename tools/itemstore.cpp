#include "itemstore.h"

#include <QGraphicsScene>

#include "tools/signalingellipse.h"
#include "tools/zoneitem.h"
#include "tools/ruleritem.h"
#include "core/projector.h"


AbstractItemStore::AbstractItemStore(QObject *parent) :
    QObject(parent)
{
}

template <class T> ItemStore<T>::ItemStore(Projector* p) :
    AbstractItemStore(parent),
    items(),
    projector(p)
{

}

template <class T> int ItemStore<T>::size() {
  return items.size();
}

template <class T> T* ItemStore<T>::at(int n) {
  if (n<size()) return items.at(n);
  return NULL;
}

template <class T> bool ItemStore<T>::delAt(const QPointF& p) {
  foreach (T* item, items) {
    if (item->contains(item->mapFromScene(p))) {
      if (item->scene()) item->scene()->removeItem(item);
      int idx = items.indexOf(item);
      items.removeAt(idx);
      delete item;
      emit itemRemoved(idx);
      return true;
    }
  }
  return false;
}

template <class T> void ItemStore<T>::clear() {
  while (!items.empty()) {
    T* item = items.takeFirst();
    if (item->scene()) item->scene()->removeItem(item);
    delete item;
    emit itemRemoved(0);
  }
  emit itemsCleared();
}

template <class T> void ItemStore<T>::emitChanged() {
  int idx = items.indexOf(sender());
  if (idx>=0) emit itemChanged(idx);
}

template <> SignalingEllipseItem* ItemStore<SignalingEllipseItem>::add(const QPointF& p) {
  SignalingEllipseItem* item = new SignalingEllipseItem(projector->imageItemsPlane);
  item->setFlag(QGraphicsItem::ItemIsMovable, true);
  item->setCursor(QCursor(Qt::SizeAllCursor));
  item->setPen(QPen(QColor(0xFF,0xAA,0x33)));
  double spotSize = 0.01*projector->getSpotSize();
  marker->setRect(QRectF(-spotSize, -spotSize, 2.0*spotSize, 2.0*spotSize));
  item->setPos(projector->det2img.map(p));
  item->setTransform(QTransform::fromScale(projector->det2img.m11(), projector->det2img.m22()));
  connect(item, SIGNAL(positionChanged()), this, SLOT(emitChanged()));
  items << item;
  emit itemAdded(size()-1);
  return item;
}

template class ItemStore<SignalingEllipseItem>;
