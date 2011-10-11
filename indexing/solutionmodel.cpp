/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

#include "solutionmodel.h"

#include <QApplication>
#include <QVector>

SolutionModel::SolutionModel(QObject* _parent):
  QAbstractTableModel(_parent)
{
}

int SolutionModel::rowCount(const QModelIndex& /*_parent*/) const {
  return solutions.count();
}

int SolutionModel::columnCount(const QModelIndex& /*_parent*/) const {
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

void SolutionModel::addSolutions(QList<Solution> newSolutions) {
  QList<double> solutionScore = getSortDataList();
  foreach (Solution s, newSolutions) {
    double solScore = getSortData(s);
    int idx=qLowerBound(solutionScore, solScore)-solutionScore.begin();
    beginInsertRows(QModelIndex(),idx,idx);
    solutionScore.insert(idx, solScore);
    solutions.insert(idx, s);
    endInsertRows();
  }
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
