#ifndef LIVEMARKERMODEL_H
#define LIVEMARKERMODEL_H

#include <QAbstractTableModel>

#include "tools/abstractmarkeritem.h"

class Crystal;

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
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  virtual void sort(int column, Qt::SortOrder order);
signals:
  void doHighlightMarker(int);
  void deleteMarker(AbstractMarkerItem*);
public slots:
  void highlightMarker(int n, bool b);
  void deleteMarker(int);
protected slots:
  void markerAdded(AbstractMarkerItem*);
  void markerChanged(AbstractMarkerItem*);
  void markerClicked(AbstractMarkerItem*);
  void markerRemoved(AbstractMarkerItem*);
  void rescore();
  void prepareDelete();
private:
  QVariant sumRowData(const QModelIndex & index, int role = Qt::DisplayRole) const;
  QList<double> getSortDataList();
  double getSortData(AbstractMarkerItem*);
  Crystal* crystal;
  QList<AbstractMarkerItem*> markers;
  int sortColumn;
  Qt::SortOrder sortOrder;
};




#endif // LIVEMARKERMODEL_H
