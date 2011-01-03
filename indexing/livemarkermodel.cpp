#include "livemarkermodel.h"

#include "core/crystal.h"
#include "core/projector.h"
#include "tools/spotitem.h"
#include "tools/zoneitem.h"

LiveMarkerModel::LiveMarkerModel(Crystal *c, QObject *parent) :
    QAbstractTableModel(parent),
    crystal(c)
{
  foreach (Projector* p, c->getConnectedProjectors()) {
    observeProjector(p);
  }
  connect(crystal, SIGNAL(projectorAdded(Projector*)), this, SLOT(observeProjector(Projector*)));
  connect(crystal, SIGNAL(projectorRemoved(Projector*)), this, SLOT(forgetProjector(Projector*)));
  connect(crystal, SIGNAL(orientationChanged()), this, SLOT(orientationChanged()));
}

void LiveMarkerModel::addMarker(AbstractMarkerItem *m) {
  beginInsertRows(QModelIndex(), markers.size(), markers.size());
  markers << m;
  cache << MarkerCacheData(m->getBestIndex());
  QObject* o = dynamic_cast<QObject*>(m);
  if (o) {
    connect(o, SIGNAL(positionChanged()), this, SLOT(markerChanged()));
  }
  endInsertRows();
}

void LiveMarkerModel::deleteMarker(AbstractMarkerItem* m) {
  int idx = markers.indexOf(m);
  beginRemoveRows(QModelIndex(), idx, idx);
  markers.removeAt(idx);
  cache.removeAt(idx);
  endRemoveRows();
}

void LiveMarkerModel::observeProjector(Projector* p) {
  foreach (SpotItem* si, p->spotMarkers()) {
    addMarker(si);
  }
  connect(&p->spotMarkers(), SIGNAL(itemAdded(int)), this, SLOT(spotMarkerAdded(int)));
  connect(&p->spotMarkers(), SIGNAL(itemAboutToBeRemoved(int)), this, SLOT(spotMarkerRemoved(int)));
  foreach (ZoneItem* zi, p->zoneMarkers()) {
    addMarker(zi);
  }
  connect(&p->zoneMarkers(), SIGNAL(itemAdded(int)), this, SLOT(zoneMarkerAdded(int)));
  connect(&p->zoneMarkers(), SIGNAL(itemAboutToBeRemoved(int)), this, SLOT(zoneMarkerRemoved(int)));
}

void LiveMarkerModel::forgetProjector(Projector* p) {
  foreach (SpotItem* si, p->spotMarkers()) {
    deleteMarker(si);
    si->disconnect(this);
  }
  p->spotMarkers().disconnect(this);
  foreach (ZoneItem* zi, p->zoneMarkers()) {
    deleteMarker(zi);
    zi->disconnect(this);
  }
  p->zoneMarkers().disconnect(this);
}


void LiveMarkerModel::spotMarkerAdded(int n) {
  ItemStore<SpotItem>* is = dynamic_cast<ItemStore<SpotItem>*>(sender());
  if (is) {
    addMarker(is->at(n));
  }
}

void LiveMarkerModel::zoneMarkerAdded(int n) {
  ItemStore<ZoneItem>* is = dynamic_cast<ItemStore<ZoneItem>*>(sender());
  if (is) {
    addMarker(is->at(n));
  }
}

void LiveMarkerModel::spotMarkerRemoved(int n) {
  ItemStore<SpotItem>* is = dynamic_cast<ItemStore<SpotItem>*>(sender());
  if (is) {
    deleteMarker(is->at(n));
  }
}

void LiveMarkerModel::zoneMarkerRemoved(int n) {
  ItemStore<ZoneItem>* is = dynamic_cast<ItemStore<ZoneItem>*>(sender());
  if (is) {
    deleteMarker(is->at(n));
  }
}

void LiveMarkerModel::markerChanged() {
  AbstractMarkerItem* item = dynamic_cast<AbstractMarkerItem*>(sender());
  int idx = markers.indexOf(item);
  cache[idx] = MarkerCacheData(item->getBestIndex());
  emit dataChanged(index(idx, 0), index(idx, columnCount()));
  // emit changed
}


void LiveMarkerModel::orientationChanged() {
  for (int n=0; n<markers.size(); n++)
    cache[n].bestIndex = markers.at(n)->getBestIndex();
  emit dataChanged(index(0, 0), index(columnCount(), rowCount()));
}


int LiveMarkerModel::rowCount(const QModelIndex &parent) const {
  return markers.size();
}

int LiveMarkerModel::columnCount(const QModelIndex &parent) const {
  return 9;
}

QVariant LiveMarkerModel::data(const QModelIndex &index, int role) const {
  if (role==Qt::DisplayRole) {
    int col = index.column();
    if (col==0 || col==1 || col==2) {
      Vec3D n = cache.at(index.row()).bestIndex;
      return QVariant(QString::number(qRound(n(col))));
    } else if (col==3 || col==4 || col==5) {
      Vec3D n = cache.at(index.row()).bestIndex;
      return QVariant(QString::number(n(col-3), 'f', 3));
    } else if (col==8) {
      Vec3D n = cache.at(index.row()).bestIndex;
      double s = 0.0;
      for (int i=0; i<3; i++) s += (n(i)-qRound(n(i)))*(n(i)-qRound(n(i)));
      return QVariant(QString::number(100.0*sqrt(s), 'f', 2));
    }
  } else if (role==Qt::BackgroundRole && false) {
    return QVariant(QBrush(QColor(225, 255, 225)));
  } else if (role==Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignRight);
  }
  return QVariant();
}

QVariant LiveMarkerModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if ((role==Qt::DisplayRole) && (orientation==Qt::Horizontal)) {
    const char* data[] = {"h", "k", "l", "h", "k", "l", "Angular", "Spatial", "HKL"};
    return QVariant(QString(data[section]));
  }
  return QVariant();
}

void LiveMarkerModel::sort(int column, Qt::SortOrder order) {

}
