#ifndef LIVEMARKERMODEL_H
#define LIVEMARKERMODEL_H

#include <QAbstractTableModel>

#include "tools/abstractmarkeritem.h"

class Crystal;
class Projector;

class LiveMarkerModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit LiveMarkerModel(Crystal*, QObject *parent = 0);
  ~LiveMarkerModel();

  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const ;
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

signals:
public slots:
  void observeProjector(Projector*);
  void forgetProjector(Projector*);
  void hightlightMarker(int n, bool b);
protected slots:
  void spotMarkerAdded(int);
  void zoneMarkerAdded(int);
  void spotMarkerRemoved(int);
  void zoneMarkerRemoved(int);
  void markerChanged();
  void orientationChanged();
protected:
  void addMarker(AbstractMarkerItem* m);
  void deleteMarker(AbstractMarkerItem* m);
private:
  Crystal* crystal;
  QList<AbstractMarkerItem*> markers;

};




#endif // LIVEMARKERMODEL_H
