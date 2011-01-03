#ifdef ITEMSTORE_H

#include <QGraphicsScene>

#include "tools/zoneitem.h"
#include "tools/ruleritem.h"



template <class T> ItemStore<T>::ItemStore(QObject* parent) :
    AbstractItemStore(parent),
    items()
{

}

template <class T> ItemStore<T>::ItemStore(const ItemStore &o) {
  items = o.items;
}

template <class T> typename ItemStore<T>::const_iterator ItemStore<T>::begin() const {
  return items.begin();
}

template <class T> typename ItemStore<T>::const_iterator ItemStore<T>::end() const {
  return items.end();
}

template <class T> int ItemStore<T>::size() {
  return items.size();
}

template <class T> T* ItemStore<T>::at(int n) {
  if (n<size()) return items.at(n);
  return NULL;
}

template <class T> T* ItemStore<T>::last() {
  if (items.isEmpty()) return NULL;
  return items.last();
}

template <class T> bool ItemStore<T>::delAt(const QPointF& p) {
  foreach (T* item, items) {
    if (item->contains(item->mapFromScene(p))) {
      if (item->scene()) item->scene()->removeItem(item);
      int idx = items.indexOf(item);
      emit itemAboutToBeRemoved(idx);
      items.removeAt(idx);
      emit itemRemoved(idx);
      delete item;
      return true;
    }
  }
  return false;
}

template <class T> void ItemStore<T>::clear() {
  while (!items.empty()) {
    emit itemAboutToBeRemoved(0);
    T* item = items.takeFirst();
    if (item->scene()) item->scene()->removeItem(item);
    emit itemRemoved(0);
    delete item;
  }
  emit itemsCleared();
}

template <class T> void ItemStore<T>::emitChanged() {
  int idx = items.indexOf(dynamic_cast<T*>(sender()));
  if (idx>=0) emit itemChanged(idx);
}

template <class T> void ItemStore<T>::addItem(T* item) {
  QObject* o = dynamic_cast<QObject*>(item);
  if (o)
    connect(o, SIGNAL(positionChanged()), this, SLOT(emitChanged()));
  items << item;
  emit itemAdded(size()-1);
}

#endif
