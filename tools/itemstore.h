#ifndef ITEMSTORE_H
#define ITEMSTORE_H

#include <QObject>
#include <QPointF>
#include <QGraphicsItem>
#include <QSignalMapper>

class ItemStore: public QObject {
  Q_OBJECT
friend class Projector;
public:
  explicit ItemStore(QObject* parent=0);
  int size();
  QGraphicsItem* at(int);
public slots:
  virtual bool delAt(const QPointF& );
  virtual void clear();
signals:
  void itemAdded(int);
  void itemChanged(int);
  void itemRemoved(int);
  void itemsCleared();
protected:
  virtual void addItem(QGraphicsItem*);
protected slots:
  virtual void emitChanged();
private:
  ItemStore(const ItemStore&) {};
  ItemStore& operator=(const ItemStore&) { return *this; }
  QList<QGraphicsItem*> items;
};

#endif // ITEMSTORE_H
