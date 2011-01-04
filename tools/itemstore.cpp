#ifdef ITEMSTORE_H
//#include "tools/itemstore.h"

#include <QGraphicsScene>
#include <QMetaMethod>
#include <iostream>

using namespace std;

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

template <class T> bool ItemStore<T>::del(int n) {
  if (n>=0 && n<items.size()) {
    emit itemAboutToBeRemoved(n);
    T* item = items.takeAt(n);
    // Don't remove Item from scene. This leads to a crash and
    // is done in the destructor anyway
    emit itemRemoved(n);
    delete item;
    return true;
  }
  return false;
}

template <class T> bool ItemStore<T>::del(T* item) {
  return del(items.indexOf(item));
}

template <class T> bool ItemStore<T>::delAt(const QPointF& p) {
  foreach (T* item, items) {
    if (item->contains(item->mapFromScene(p))) {
      return del(item);
    }
  }
  return false;
}

template <class T> void ItemStore<T>::clear() {
  while (!items.empty())
    del(0);
  emit itemsCleared();
}

template <class T> void ItemStore<T>::emitChanged() {
  int idx = items.indexOf(dynamic_cast<T*>(sender()));
  if (idx>=0) emit itemChanged(idx);
}

template <class T> void ItemStore<T>::addItem(T* item) {
  QObject* o = dynamic_cast<QObject*>(item);
  if (o) {
    connect(o, SIGNAL(positionChanged()), this, SLOT(emitChanged()));
    for (int i=0; i<o->metaObject()->methodCount(); i++) {
      if (o->metaObject()->method(i).methodType()==QMetaMethod::Signal) {
        if (QString(o->metaObject()->method(i).signature())=="itemClicked()") {
          connect(o, SIGNAL(itemClicked()), this, SLOT(emitItemClicked()));
        }
      }
    }
  }
  items << item;
  emit itemAdded(size()-1);
}

template <class T> void ItemStore<T>::emitItemClicked() {
  int idx = items.indexOf(dynamic_cast<T*>(sender()));
  if (idx>=0)
    emit itemClicked(idx);
}

#endif
