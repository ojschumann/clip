#ifndef SOLUTIONMODEL_H
#define SOLUTIONMODEL_H

#include <QAbstractTableModel>

#include "tools/vec3D.h"
#include "tools/mat3D.h"
#include "indexing/solution.h"

class SolutionModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  SolutionModel(QObject* parent=0);
  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const ;
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) ;


  Solution getSolution(unsigned int n);

signals:
  void solutionNumberChanged(int);
public slots:
  void addSolution(Solution s);
  void clear();

private:
  class SolSort {
  public:
    SolSort(int col, Qt::SortOrder order);
    bool operator()(const Solution& s1,const Solution& s2);
            private:
    int sortColumn;
    Qt::SortOrder sortOrder;
  };


  QList<Solution> solutions;
  int sortColumn;
  Qt::SortOrder sortOrder;
};



#endif // SOLUTIONMODEL_H
