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

#include "fitparametertreeitem.h"

 
#include <cmath>

using namespace std;

FitParameterTreeItem::FitParameterTreeItem(FitParameter *p, QTreeWidgetItem* parent):
    QObject(0),
    QTreeWidgetItem(parent),
    parameter(p)
{
  setHidden(!p->isChangeable());
  setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable|Qt::ItemIsEditable);
  connect(p, SIGNAL(valueChanged(FitParameter*,double)), this, SLOT(updateData()));
  connect(p, SIGNAL(enabledStateChanged(FitParameter*,bool)), this, SLOT(updateData()));
  connect(p, SIGNAL(changeableStateChanged(FitParameter*,bool)), this, SLOT(changeableStateChanged(FitParameter*,bool)));
}

QVariant FitParameterTreeItem::data(int column, int role) const {
  if (column==0) {
    if (role==Qt::DisplayRole) {
      return QVariant(parameter->name());
    } else if (role==Qt::CheckStateRole) {
      return QVariant(parameter->isEnabled()?Qt::Checked:Qt::Unchecked);
    }
  } else if (column==1) {
    if (role==Qt::DisplayRole) {
      int digits = std::max(0, -static_cast<int>(floor(log10(parameter->epsilon()))));
      return QVariant(QString::number(parameter->value(), 'f', digits));
    } else if (role==Qt::EditRole) {
      return QVariant(QString::number(parameter->value()));
    }
  } else if (column==2) {
    if (role==Qt::DisplayRole) {
      if (parameter->error()>0.0) {
        int digits = std::max(0, -static_cast<int>(floor(log10(parameter->epsilon()))));
        return QVariant(QString::number(parameter->error(), 'f', digits));
      }
    }
  }
  return QVariant();
}

void FitParameterTreeItem::setData(int column, int role, const QVariant &value) {
  if ((column==0) && (role==Qt::CheckStateRole)) {
    parameter->setEnabled(value.toBool());
    emitDataChanged();
  } else if ((column==1) && (role==Qt::EditRole)) {
    bool ok;
    double v = value.toDouble(&ok);
    if (ok) {
      parameter->prepareValue(v);
      parameter->setValue();
    }
  }
}

void FitParameterTreeItem::updateData() {
  emitDataChanged();
}

void FitParameterTreeItem::changeableStateChanged(FitParameter*, bool b) {
  setHidden(!b);
}

