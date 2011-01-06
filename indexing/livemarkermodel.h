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
  virtual ~LiveMarkerModel();

  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const ;
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

signals:
  void doHighlightMarker(int);
public slots:
  void observeProjector(Projector*);
  void forgetProjector(Projector*);
  void highlightMarker(int n, bool b);
  void deleteMarker(int);
protected slots:
  void spotMarkerAdded(int);
  void zoneMarkerAdded(int);
  void spotMarkerRemoved(int);
  void zoneMarkerRemoved(int);
  void markerChanged();
  void orientationChanged();
  void markerClicked();
protected:
  void addMarker(AbstractMarkerItem* m);
  void deleteMarker(AbstractMarkerItem* m);
private:
  Crystal* crystal;
  QList<AbstractMarkerItem*> markers;

};




#endif // LIVEMARKERMODEL_H
