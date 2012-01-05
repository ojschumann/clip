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

double SolutionModel::columnDataFromSolution(const Solution& s, int column) {
  if (column==0) {
    return 100.0*s.hklDeviationSqSum();
  } else if (column==1) {
    return s.allIndexRMS();
  } else if (column==2) {
    return 100.0*s.hklDeviationSqSum()*s.allIndexRMS();
  }
  return 0.0;
}

QVariant SolutionModel::data(const QModelIndex & index, int role) const {
  if (role==Qt::DisplayRole) {
    return QVariant(QString::number(columnDataFromSolution(solutions.at(index.row()), index.column()), 'f', 2));
  } else if (role==Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignRight);
  }
  return QVariant();
}

QVariant SolutionModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role==Qt::DisplayRole) {
    if (orientation==Qt::Horizontal) {
      if (section==0) {
        return QVariant("Idx Dev");
      } else if (section==1) {
        return QVariant("Idx rms");
      } else if (section==2) {
        return QVariant("Combination");
      }
    } else {
      return QVariant(section+1);
    }
  } else if (role==Qt::ToolTipRole && orientation==Qt::Horizontal) {
    if (section==0) {
      return QVariant("<html>Root mean square of the fractional part of the rational index componentes. Favors good matching solutions.</html>");
    } else if (section==1) {
      return QVariant("<html>Root mean square of the index components. Favors low indexed solutions.</html>");
    } else if (section==2) {
      return QVariant("<html>Combination of <i>Idx&nbsp;Dev</i> and <i>Idx&nbsp;rms</i></html>");
    }
  }
  return QVariant();
}

void SolutionModel::sort(int column, Qt::SortOrder order) {
  sortColumn = column;
  sortOrder = order;

  for (int i=0; i<solutions.size(); i++)
    solutions[i].solutionIndex = i;

  emit layoutAboutToBeChanged();

  qStableSort(solutions.begin(), solutions.end(), SolutionCompare(sortColumn, sortOrder));

  QVector<int> ids(solutions.size());
  for (int i=0; i<solutions.size(); i++) {
    ids[solutions.at(i).solutionIndex]=i;
  }

  QModelIndexList oldPersistentIndices = persistentIndexList();
  QModelIndexList newPersistentIndices;
  foreach (QModelIndex idx, oldPersistentIndices) {
    newPersistentIndices << index(ids.at(idx.row()), idx.column(), idx.parent());
  }
  changePersistentIndexList(oldPersistentIndices, newPersistentIndices);
  //emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
  emit layoutChanged();
}


void SolutionModel::addSolution(Solution s) {
  int idx=qLowerBound(solutions.begin(), solutions.end(), s, SolutionCompare(sortColumn, sortOrder))-solutions.begin();
  beginInsertRows(QModelIndex(),idx,idx);
  solutions.insert(idx, s);
  endInsertRows();
  emit solutionNumberChanged(solutions.size());
}

void SolutionModel::addSolutions(QList<Solution> newSolutions) {
  foreach (Solution s, newSolutions) {
    int idx=qLowerBound(solutions.begin(), solutions.end(), s, SolutionCompare(sortColumn, sortOrder))-solutions.begin();
    beginInsertRows(QModelIndex(),idx,idx);
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

SolutionModel::SolutionCompare::SolutionCompare(int column, Qt::SortOrder order) {
  sortColumn=column;
  sortOrder=order;
};

bool SolutionModel::SolutionCompare::operator()(const Solution& s1,const Solution& s2) {
  double v1 = SolutionModel::columnDataFromSolution(s1, sortColumn);
  double v2 = SolutionModel::columnDataFromSolution(s2, sortColumn);
  if (sortOrder==Qt::AscendingOrder)
    return v1<v2;
  return v2<v1;
}
