#ifndef ITEMSTORE_H
#define ITEMSTORE_H

#include <QObject>
#include <QPointF>
#include <QSignalMapper>

class Projector;

class AbstractItemStore : public QObject
{
  Q_OBJECT
public:
  explicit AbstractItemStore(QObject *parent = 0);
signals:
  void itemAdded(int);
  void itemChanged(int);
  void itemRemoved(int);
  void itemsCleared();
public slots:
  void emitChanged();
  virtual bool delAt(const QPointF& )=0;
  virtual void clear()=0;
};


template <class T> class ItemStore: public AbstractItemStore {
public:
  explicit ItemStore(Projector* p);
  int size();
  T* at(int);
  virtual T* add(const QPointF&) { return NULL; }
  virtual T* add(const QPointF&, const QPointF&) { return NULL; }
  virtual T* add(const QPointF&, const QString&) { return NULL; }
  virtual bool delAt(const QPointF& );
  virtual void clear();
private:
  QList<T*> items;
  Projector* projector;
};

class SignalingEllipseItem;
template<> SignalingEllipseItem* ItemStore<SignalingEllipseItem>::add(const QPointF&);


#endif // ITEMSTORE_H
