#ifndef MARKERMODEL_H
#define MARKERMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include "indexing/solution.h"


class MarkerModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit MarkerModel(QObject *parent = 0);

  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const ;
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

signals:

public slots:
  void setSolution(Solution);
  void clearDisplay();
private:
  Solution solution;
  bool solutionValid;
};

#endif // MARKERMODEL_H
