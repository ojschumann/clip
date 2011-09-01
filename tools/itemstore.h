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
  typedef T* value_type;

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
  ItemStore<T>& operator=(const ItemStore<T>&);
  QList<T*> items;
};

#include "itemstore.cpp"

#endif // ITEMSTORE_H
