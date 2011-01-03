#ifndef RULERMODEL_H
#define RULERMODEL_H

#include <QAbstractTableModel>

#include <QPointer>
#include "tools/itemstore.h"

class RulerItem;

class RulerModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit RulerModel(ItemStore<RulerItem>& r, QObject* parent=0);
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
public slots:
  void slotRulerAdded();
  void itemChanged(int);
  void setResolution(double, double);
protected:
  ItemStore<RulerItem>& rulers;
  double hRes;
  double vRes;
};

#endif // RULERMODEL_H
