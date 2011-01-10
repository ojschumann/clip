#include "fitparametertreeitem.h"

#include <iostream>

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
      return QVariant(parameter->value());
    } else if (role==Qt::EditRole) {
      return QVariant(QString::number(parameter->value()));
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

