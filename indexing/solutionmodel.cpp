#include "solutionmodel.h"

#include <QApplication>
#include <QVector>

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
      return QVariant(QString::number(100.0*solutions.at(index.row()).hklDeviationSum(), 'f', 2));
    } else if (index.column()==1) {
      return QVariant(QString::number(solutions.at(index.row()).allIndexMean(), 'f', 2));
    } else if (index.column()==2) {
      return QVariant(QString::number(solutions.at(index.row()).allIndexRMS(), 'f', 2));
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
        return QVariant("Deviation");
      } else if (section==1) {
        return QVariant("Idx mean");
      } else if (section==2) {
        return QVariant("Idx rms");
      }
    } else {
      return QVariant(section+1);
    }
  }
  return QVariant();
}

QList<double> SolutionModel::getSortDataList() const {
  QList<double> list;
  for (int row=0; row<solutions.size(); row++) {
    list << getSortData(solutions.at(row));
  }
  return list;
}

double SolutionModel::getSortData(const Solution& s) const {
  double sign = (sortOrder==Qt::AscendingOrder) ? 1.0 : -1.0;
  if (sortColumn==0) {
    return sign*s.hklDeviationSum();
  } else if (sortColumn==1) {
    return sign*s.allIndexMean();
  } else if (sortColumn==2) {
    return sign*s.allIndexRMS();
  }
  return 0.0;
}


struct SortFunctor {
  SortFunctor(double v, int i): value(v), id(i) {}
  bool operator<(const SortFunctor& o) const { return value<o.value; }
  double value;
  int id;
};

void SolutionModel::sort(int column, Qt::SortOrder order) {
  sortColumn = column;
  sortOrder = order;

  QList<SortFunctor> values;
  QList<double> v = getSortDataList();
  for (int i=0; i<v.size(); i++) {
    values << SortFunctor(v.at(i), i);
  }

  qStableSort(values);

  QVector<int> ids(values.size());
  QList<Solution> tmpSolutions;
  for (int i=0; i<solutions.size(); i++) {
    tmpSolutions << solutions.at(values.at(i).id);
    ids[values.at(i).id]=i;
  }
  solutions = tmpSolutions;

  QModelIndexList newIndices;
  foreach (QModelIndex idx, persistentIndexList()) {
    newIndices << index(ids.at(idx.row()), idx.column(), idx.parent());
  }
  changePersistentIndexList(persistentIndexList(), newIndices);
  emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}


void SolutionModel::addSolution(Solution s) {
  QList<double> solutionScore = getSortDataList();
  int idx=qLowerBound(solutionScore, getSortData(s))-solutionScore.begin();
  beginInsertRows(QModelIndex(),idx,idx);
  solutions.insert(idx, s);
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
    b=s1.allIndexMean()<s2.allIndexMean();
  } else if (sortColumn==1) {
    b=s1.allIndexRMS()<s2.allIndexRMS();
  } else if (sortColumn==2) {
    b=s1.hklDeviationSum()<s2.hklDeviationSum();
  }
  if (sortOrder==Qt::DescendingOrder)
    b=not b;
  return b;
}
