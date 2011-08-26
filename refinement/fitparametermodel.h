/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

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

#ifndef FITPARAMETERMODEL_H
#define FITPARAMETERMODEL_H

#include <QAbstractItemModel>
#include <QSignalMapper>

class Crystal;
class FitObject;
class FitParameter;
class Projector;

class FitParameterModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit FitParameterModel(Crystal* c, QObject *parent = 0);
  virtual ~FitParameterModel();
  virtual int columnCount(const QModelIndex &parent) const;
  virtual int rowCount(const QModelIndex &parent) const;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
  virtual QModelIndex parent(const QModelIndex &child) const;

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

  FitObject* node(const QModelIndex& index) const;
  FitParameter* parameter(const QModelIndex& index) const;

protected slots:
  void handleProjectorAdd(Projector*);
  void handleProjectorDel(Projector*);

  void handleMarkerAdd();
  void handleMarkerDel();

  void parameterValueChanged(FitParameter*, double);

protected:
  Crystal* crystal;
  QList<FitObject*> nodes;
  QSignalMapper mapper;

};

#endif // FITPARAMETERMODEL_H
