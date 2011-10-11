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
  SolutionModel(QObject* _parent=nullptr);
  virtual int rowCount(const QModelIndex & _parent= QModelIndex()) const;
  virtual int columnCount(const QModelIndex & _parent= QModelIndex()) const ;
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) ;


  Solution getSolution(unsigned int n);

signals:
  void solutionNumberChanged(int);
public slots:
  void addSolution(Solution s);
  void addSolutions(QList<Solution> newSolutions);
  void clear();

private:
  QList<double> getSortDataList() const;
  double getSortData(const Solution& s) const;

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
