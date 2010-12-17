#include "solutionmodel.h"

#include <QApplication>

SolutionModel::SolutionModel(QObject *parent):
  QAbstractTableModel(parent)
{
}

int SolutionModel::rowCount(const QModelIndex & parent) const {
  return solutions.count();
}

int SolutionModel::columnCount(const QModelIndex & parent) const {
  return 3;
}

QVariant SolutionModel::data(const QModelIndex & index, int role) const {
  if (role==Qt::DisplayRole) {
    if (index.column()==0) {
      return QVariant(solutions.at(index.row()).spatialDeviationSum());
    } else if (index.column()==1) {
      return QVariant(solutions.at(index.row()).angularDeviationSum());
    } else if (index.column()==2) {
      return QVariant(solutions.at(index.row()).hklDeviationSum());
    }
  } else if (role==Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignRight);
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
  qSort(solutions.begin(), solutions.end(), SolSort(sortColumn, sortOrder));
  reset();
}


void SolutionModel::addSolution(Solution s) {
  QList<Solution>::iterator iter=qLowerBound(solutions.begin(), solutions.end(), s, SolSort(sortColumn, sortOrder));
  int n=iter-solutions.begin();
  beginInsertRows(QModelIndex(),n,n);
  solutions.insert(iter,s);
  endInsertRows();
  emit solutionNumberChanged(solutions.size());
}

void SolutionModel::clear() {
  solutions.clear();
  emit solutionNumberChanged(solutions.size());
  reset();
}

Solution SolutionModel::getSolution(unsigned int n) {
  return solutions[n];
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
