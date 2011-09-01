/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

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
  void maxSearchIndexChanged(int);
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
