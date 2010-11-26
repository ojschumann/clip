#include "markermodel.h"

#include "core/crystal.h"
#include "core/projector.h"

MarkerModel::MarkerModel(Crystal *_c, QObject *parent) :
    QAbstractTableModel(parent),
    crystal(_c)
{
  loadMarkerNormals();
}


int MarkerModel::rowCount(const QModelIndex &parent) const {
  return spotMarker.size();
}

int MarkerModel::columnCount(const QModelIndex &parent) const {
  return 0;
}

QVariant MarkerModel::data(const QModelIndex &index, int role) const {
  return QVariant();
}

QVariant MarkerModel::headerData(int section, Qt::Orientation orientation, int role) const {
  return QVariant();
}

void MarkerModel::sort(int column, Qt::SortOrder order) {

}

void MarkerModel::loadMarkerNormals() {
  foreach (Projector* p, crystal->getConnectedProjectors()) {
    spotMarker += p->getSpotMarkerNormals();
    zoneMarker += p->getZoneMarkerNormals();
  }
}
