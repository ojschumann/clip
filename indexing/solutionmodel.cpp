#include "solutionmodel.h"

SolutionModel::SolutionModel(QObject* parent): QAbstractTableModel(parent) {
}

int SolutionModel::rowCount(const QModelIndex & parent) const {
  return solution.count();
}

int SolutionModel::columnCount(const QModelIndex & parent) const {
  return 3;
}

QVariant SolutionModel::data(const QModelIndex & index, int role) const {
  if (role==Qt::DisplayRole) {
    if (index.column()==0) {
      return QVariant(solution.at(index.row()).spatialDeviationSum());
    } else if (index.column()==1) {
      return QVariant(solution.at(index.row()).angularDeviationSum());
    } else if (index.column()==2) {
      return QVariant(solution.at(index.row()).hklDeviationSum());
    }
  }
  return QVariant();
}

QVariant SolutionModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role==Qt::DisplayRole) {
    if (orientation==Qt::Horizontal) {
      if (section==0) {
        return QVariant("Spacial");
      } else if (section==1) {
        return QVariant("Angular");
      } else if (section==2) {
        return QVariant("HKL");
      }
    } else {
      return QVariant(section+1);
    }
  }
  return QVariant();
}


void SolutionModel::sort(int column, Qt::SortOrder order) {
  sortColumn=column;
  sortOrder=order;
  qSort(solution.begin(), solution.end(), SolSort(sortColumn, sortOrder));
  reset();
}

void SolutionModel::startIndexing(SolutionModel::IndexingParameter& _p) {
/*  emit stopWorker();
  p=_p;
  cout << p.pointGroup.size() << endl;
  solution.clear();
  reset();
  IndexWorker* worker=new IndexWorker(p);
  qRegisterMetaType<Solution>();
  connect(worker, SIGNAL(publishSolution(Solution)), this, SLOT(addSolution(Solution)));
  connect(worker, SIGNAL(progressInfo(int, int)), this, SIGNAL(progressInfo(int,int)));
  connect(worker, SIGNAL(destroyed()), this, SLOT(threadFinished()));
  connect(this, SIGNAL(stopWorker()), worker, SLOT(stop()));
  connect(this, SIGNAL(destroyed()), worker, SLOT(stop()));
  QThreadPool::globalInstance()->start(worker);
  emit runningStateChanged(true);
  */
}

void SolutionModel::addSolution(Solution s) {
  QList<Solution>::iterator iter=qLowerBound(solution.begin(), solution.end(), s, SolSort(sortColumn, sortOrder));
  int n=iter-solution.begin();
  beginInsertRows(QModelIndex(),n,n);
  solution.insert(iter,s);
  endInsertRows();
}

Solution SolutionModel::getSolution(unsigned int n) {
  return solution[n];
}

void SolutionModel::threadFinished() {
  emit runningStateChanged(false);
}


SolutionModel::SolSort::SolSort(int col, Qt::SortOrder order) {
  sortColumn=col;
  sortOrder=order;
};

bool SolutionModel::SolSort::operator()(const Solution& s1,const Solution& s2) {
  bool b=true;
  if (sortColumn==0) {
    b=s1.angularDeviationSum()<s2.angularDeviationSum();
  } else if (sortColumn==1) {
    b=s1.spatialDeviationSum()<s2.spatialDeviationSum();
  } else if (sortColumn==2) {
    b=s1.hklDeviationSum()<s2.hklDeviationSum();
  }
  if (sortOrder==Qt::DescendingOrder)
    b=not b;
  return b;
}
