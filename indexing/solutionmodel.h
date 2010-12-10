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
  struct IndexingParameter {
    QList<Vec3D> markerNormals;
    //QList<Reflection> refs;
    QList<Mat3D> pointGroup;
    double maxAngularDeviation;
    double maxIntegerDeviation;
    unsigned int maxOrder;
    Mat3D orientationMatrix;
  };

  SolutionModel(QObject* parent=0);
  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const ;
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) ;


  void startIndexing(IndexingParameter& p);
  Solution getSolution(unsigned int n);

public slots:
  void addSolution(Solution s);
  void threadFinished();

signals:
  void stopWorker();
  void runningStateChanged(bool);
  void progressInfo(int, int);
private:
  class SolSort {
  public:
    SolSort(int col, Qt::SortOrder order);
    bool operator()(const Solution& s1,const Solution& s2);
            private:
    int sortColumn;
    Qt::SortOrder sortOrder;
  };


  QList<Solution> solution;
  int sortColumn;
  Qt::SortOrder sortOrder;
  IndexingParameter p;
};



#endif // SOLUTIONMODEL_H
