#ifndef MARKERMODEL_H
#define MARKERMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include "tools/vec3D.h"

class Crystal;

class MarkerModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit MarkerModel(Crystal* _c, QObject *parent = 0);

  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const ;
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  void loadMarkerNormals();
signals:

public slots:

private:
  Crystal* crystal;
  QList<Vec3D> spotMarker;
  QList<Vec3D> zoneMarker;

};

#endif // MARKERMODEL_H
