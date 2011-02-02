#ifndef RULERMODEL_H
#define RULERMODEL_H

#include <QAbstractTableModel>

#include <QPointer>
#include "tools/itemstore.h"
#include "image/laueimage.h"

class RulerItem;

class RulerModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit RulerModel(ItemStore<RulerItem>& r, LaueImage* img, QObject* parent=0);
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
public slots:
  void rulerAdded();
  void rulerChanged(int);
  void rulerRemoved(int);
  void setResolution(double, double);
protected:
  ItemStore<RulerItem>& rulers;
  LaueImage* image;
  double hRes;
  double vRes;
};

#endif // RULERMODEL_H
