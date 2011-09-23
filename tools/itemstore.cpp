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

#ifdef ITEMSTORE_H
//#include "tools/itemstore.h"

#include <QGraphicsScene>
#include <QMetaMethod>
 

using namespace std;

template <class T> ItemStore<T>::ItemStore(QObject* _parent) :
    AbstractItemStore(_parent),
    items()
{

}

template <class T> ItemStore<T>::ItemStore(const ItemStore &o):AbstractItemStore() {
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
  return nullptr;
}

template <class T> T* ItemStore<T>::last() {
  if (items.isEmpty()) return nullptr;
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
