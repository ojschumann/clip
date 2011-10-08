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

#ifndef RULERMODEL_H
#define RULERMODEL_H

#include <QAbstractTableModel>

#include <QPointer>
#include "tools/itemstore.h"
#include "image/laueimage.h"

class RulerItem;

class RulerModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit RulerModel(ItemStore<RulerItem>& r, LaueImage* img, QObject* _parent=nullptr);
  int rowCount(const QModelIndex &_parent= QModelIndex()) const;
  int columnCount(const QModelIndex &_parent= QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
public slots:
  void rulerAdded();
  void rulerChanged(int);
  void rulerRemoved(int);
  void setResolution(double, double);
protected:
  ItemStore<RulerItem>& rulers;
  LaueImage* image;
  double hRes;
  double vRes;
};

#endif // RULERMODEL_H
