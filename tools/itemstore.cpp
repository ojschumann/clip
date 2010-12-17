#include "itemstore.h"

#include <QGraphicsScene>

#include "tools/zoneitem.h"
#include "tools/ruleritem.h"



ItemStore::ItemStore(QObject* parent) :
    QObject(parent),
    items()
{

}

int ItemStore::size() {
  return items.size();
}

QGraphicsItem* ItemStore::at(int n) {
  if (n<size()) return items.at(n);
  return NULL;
}

bool ItemStore::delAt(const QPointF& p) {
  foreach (QGraphicsItem* item, items) {
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

void ItemStore::clear() {
  while (!items.empty()) {
    QGraphicsItem* item = items.takeFirst();
    if (item->scene()) item->scene()->removeItem(item);
    delete item;
    emit itemRemoved(0);
  }
  emit itemsCleared();
}

void ItemStore::emitChanged() {
  QGraphicsObject* o = dynamic_cast<QGraphicsObject*>(sender());
  int idx = items.indexOf(o);
  if (idx>=0) emit itemChanged(idx);
}

void ItemStore::addItem(QGraphicsItem* item) {
  QGraphicsObject* o = dynamic_cast<QGraphicsObject*>(item);
  if (o)
    connect(o, SIGNAL(positionChanged()), this, SLOT(emitChanged()));
  items << item;
  emit itemAdded(size()-1);
}
