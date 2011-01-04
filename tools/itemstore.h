#ifndef ITEMSTORE_H
#define ITEMSTORE_H

#include <QObject>
#include <QPointF>
#include <QGraphicsItem>
#include <QSignalMapper>

class AbstractItemStore: public QObject {
  Q_OBJECT
public:
  AbstractItemStore(QObject* parent=0): QObject(parent) {}
  virtual ~AbstractItemStore() {}
signals:
  void itemAdded(int);
  void itemChanged(int);
  void itemAboutToBeRemoved(int);
  void itemRemoved(int);
  void itemClicked(int);
  void itemsCleared();
public slots:
  virtual bool del(int)=0;
  virtual bool delAt(const QPointF&)=0;
  virtual void clear()=0;
protected slots:
  virtual void emitChanged()=0;
  virtual void emitItemClicked()=0;
};

template <class T> class ItemStore: public AbstractItemStore {
friend class Projector;
public:
  explicit ItemStore(QObject* parent=0);
  ItemStore(const ItemStore<T>&);
  virtual ~ItemStore() {}
  int size();
  T* at(int);
  T* last();
  bool del(T*);
  typedef typename QList<T*>::const_iterator const_iterator;
  const_iterator begin() const;
  const_iterator end() const;
// Slots
  virtual bool del(int n);
  virtual bool delAt(const QPointF& );
  virtual void clear();
protected:
  virtual void addItem(T*);
//protected slots:
  virtual void emitChanged();
  virtual void emitItemClicked();
private:
  ItemStore<T>& operator=(const ItemStore<T>&) { return *this; }
  QList<T*> items;
};

#include "itemstore.cpp"

#endif // ITEMSTORE_H
