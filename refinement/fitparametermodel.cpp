#include "fitparametermodel.h"

#include <limits>

#include "core/crystal.h"
#include "core/projector.h"

FitParameterModel::FitParameterModel(Crystal* c, QObject *parent) :
    QAbstractItemModel(parent),
    crystal(c)
{
  nodes << crystal;
  foreach (Projector* p, crystal->getConnectedProjectors()) {
    if (p->hasMarkers() && p->allParameters().size()>0)
      nodes << p;
  }
  // ToDo:
  // Handle addition and deletion of nodes;
  // Handle change of Changable flag for FitParameters
  // Handle addition and deletion of Projector
  // Handle Addition and deletion of markers (might hide a detector)

  connect(crystal, SIGNAL(projectorAdded(Projector*)), this, SLOT(handleProjectorAdd(Projector*)));
  connect(crystal, SIGNAL(projectorRemoved(Projector*)), this, SLOT(handleProjectorDel(Projector*)));
}

FitParameterModel::~FitParameterModel() {}

void FitParameterModel::handleProjectorAdd(Projector* p) {
  connect(p, SIGNAL(markerAdded()), this, SLOT(handleMarkerAdd()));
  connect(p, SIGNAL(markerRemoved())), this, SLOT(handleMarkerDel()));

  if (p->hasMarkers()) {
    int idx = nodes.size();
    beginInsertRows(QModelIndex(), idx, idx);
    nodes << p;
    endInsertRows();
  }
}

void FitParameterModel::handleProjectorDel(Projector* p) {
  p->spotMarkers().disconnect(this);
  p->zoneMarkers().disconnect(this);

  FitObject* o = dynamic_cast<FitObject*>(p);
  int idx = nodes.indexOf(o);
  if (idx>0) {
    beginRemoveRows(QModelIndex(), idx, idx);
    nodes.removeAt(idx);
    endRemoveRows();
  }

}

void FitParameterModel::handleMarkerAdd() {
  Projector* o = dynamic_cast<Projector*>(sender(o));
  if (o && !nodes.contains(o)) {
    int idx = nodes.size();
    beginInsertRows(QModelIndex(), idx, idx);
    nodes << o;
    endInsertRows();
  }
}

void FitParameterModel::handleMarkerDel() {}


int FitParameterModel::columnCount(const QModelIndex &parent) const {
  return 2;
}

int FitParameterModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    if (parent.internalId()==0) {
      return nodes[parent.row()]->allParameters().size();
    } else {
      return 0;
    }
  } else {
    return nodes.size();
  }
}

// Top nodes (Crystal + Projectors) have InternalID = 0
// FitParams have InternalID == (row of parent)+1

QModelIndex FitParameterModel::index(int row, int column, const QModelIndex &parent) const {
  if (parent.isValid()) {
    return createIndex(row, column, parent.row()+1);
  } else {
    return createIndex(row, column, 0);
  }
}

QModelIndex FitParameterModel::parent(const QModelIndex &child) const {
  if (child.internalId()==0) {
    return QModelIndex();
  } else {
    return createIndex(child.internalId()-1, 0, 0);
  }
}

FitObject* FitParameterModel::node(const QModelIndex &index) const {
  if (index.isValid()) {
    if (index.internalId()==0) return nodes[index.row()];
  } else {
    return nodes[index.internalId()-1];
  }
  return 0;
}

FitParameter* FitParameterModel::parameter(const QModelIndex &index) const {
  if (index.isValid() && index.internalId()>0) {
    return nodes[index.internalId()-1]->allParameters().at(index.row());
  }
  return 0;
}

QVariant FitParameterModel::data(const QModelIndex &index, int role) const {
  if (index.internalId()==0) { // Top level widget, Display
    if ((role==Qt::DisplayRole) && (index.column()==0)) return QVariant(nodes[index.row()]->FitObjectName());
  } else { // FitParameter
    FitParameter* p = parameter(index);
    if (role==Qt::DisplayRole) {
      if (index.column()==0) {
        return QVariant(p->name());
      } else if (index.column()==1) {
        return QVariant(p->value());
      }
    } else if (role==Qt::CheckStateRole ) {
      if (index.column()==0) return QVariant(p->isEnabled()?Qt::Checked:Qt::Unchecked);
    }
  }
  return QVariant();
}

Qt::ItemFlags FitParameterModel::flags(const QModelIndex &index) const {
  Qt::ItemFlags flag = 0;
  if (!index.isValid()) return flag;
  if (index.internalId()==0) {
    flag |= Qt::ItemIsEnabled;
  } else {
    FitParameter* p = parameter(index);
    if (p->isChangeable()) flag |= Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  }
  return flag;
}

bool FitParameterModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role==Qt::CheckStateRole && index.internalId()>0) {
    FitParameter* p = parameter(index);
    p->setEnabled(value.toBool());
    emit dataChanged(index, index);
    return true;
  }
  return false;
}
