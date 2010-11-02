#ifndef RULERMODEL_H
#define RULERMODEL_H

#include <QAbstractTableModel>

class Projector;

class RulerModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit RulerModel(Projector* p, QObject* parent=0);
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
  Projector* projector;
  double hRes;
  double vRes;
};

#endif // RULERMODEL_H
