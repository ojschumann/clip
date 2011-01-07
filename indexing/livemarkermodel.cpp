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

LiveMarkerModel::~LiveMarkerModel() {
  foreach (AbstractMarkerItem* item, markers)
    item->highlight(false);
}

void LiveMarkerModel::addMarker(AbstractMarkerItem *m) {
  beginInsertRows(QModelIndex(), markers.size(), markers.size());
  markers << m;
  QObject* o = dynamic_cast<QObject*>(m);
  if (o) {
    connect(o, SIGNAL(positionChanged()), this, SLOT(markerChanged()));
    connect(o, SIGNAL(itemClicked()), this, SLOT(markerClicked()));
  }
  endInsertRows();
}

void LiveMarkerModel::deleteMarker(AbstractMarkerItem* m) {
  int idx = markers.indexOf(m);
  beginRemoveRows(QModelIndex(), idx, idx);
  markers.removeAt(idx);
  endRemoveRows();
  if (SpotItem* si=dynamic_cast<SpotItem*>(m)) si->disconnect(this);
  if (ZoneItem* zi=dynamic_cast<ZoneItem*>(m)) zi->disconnect(this);
}

void LiveMarkerModel::observeProjector(Projector* p) {
  foreach (AbstractMarkerItem* m, p->getAllMarkers()) {
    addMarker(m);
    markersOfProjector.insert(p, m);
  }
  connect(&p->spotMarkers(), SIGNAL(itemAdded(int)), this, SLOT(spotMarkerAdded(int)));
  connect(&p->spotMarkers(), SIGNAL(itemAboutToBeRemoved(int)), this, SLOT(spotMarkerRemoved(int)));
  connect(&p->zoneMarkers(), SIGNAL(itemAdded(int)), this, SLOT(zoneMarkerAdded(int)));
  connect(&p->zoneMarkers(), SIGNAL(itemAboutToBeRemoved(int)), this, SLOT(zoneMarkerRemoved(int)));
}

void LiveMarkerModel::forgetProjector(Projector* p) {
  foreach (AbstractMarkerItem* m, p->getAllMarkers()) {
    deleteMarker(m);
  }
  p->spotMarkers().disconnect(this);
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
  item->invalidateCache();
  int idx = markers.indexOf(item);
  emit dataChanged(index(idx, 0), index(idx, columnCount()-1));
}

void LiveMarkerModel::orientationChanged() {
  double score = 0.0;
  foreach (AbstractMarkerItem* item, markers) {
    score += item->getBestScore();
  }
  cout << "Sum of Score = " << score << endl;

  emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}

int LiveMarkerModel::rowCount(const QModelIndex &parent) const {
  return markers.size();
}

int LiveMarkerModel::columnCount(const QModelIndex &parent) const {
  return 10;
}

QVariant LiveMarkerModel::data(const QModelIndex &index, int role) const {
  if (role==Qt::DisplayRole) {
    int col = index.column();
    if (col==0) {
      if (markers.at(index.row())->getType()==AbstractMarkerItem::SpotMarker) {
        return QVariant("Spot");
      } else if (markers.at(index.row())->getType()==AbstractMarkerItem::ZoneMarker) {
        return QVariant("Zone");
      }
    } else if (col==1 || col==2 || col==3) {
      TVec3D<int> n = markers.at(index.row())->getIntegerIndex();
      return QVariant(n(col-1));
    } else if (col==4 || col==5 || col==6) {
      Vec3D n = markers.at(index.row())->getRationalIndex();      
      return QVariant(QString::number(n(col-4), 'f', 2));
    } else if (col==9) {
      return QVariant(QString::number(100.0*markers.at(index.row())->getBestScore(), 'f', 2));
    }
  } else if (role==Qt::BackgroundRole && false) {
    return QVariant(QBrush(QColor(225, 255, 225)));
  } else if (role==Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignRight);
  } else if (role==Qt::UserRole) {
    int col = index.column();
    if (col==0) {
      return data(index, Qt::DisplayRole);
    } else if (col==1 || col==2 || col==3) {
      TVec3D<int> n = markers.at(index.row())->getIntegerIndex();
      return QVariant(n(col-1));
    } else if (col==4 || col==5 || col==6) {
      Vec3D n = markers.at(index.row())->getRationalIndex();
      return QVariant(n(col-4));
    } else if (col==9) {
      return QVariant(markers.at(index.row())->getBestScore());
    }
  }
  return QVariant();
}

QVariant LiveMarkerModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if ((role==Qt::DisplayRole) && (orientation==Qt::Horizontal)) {
    const char* data[] = {"T", "h", "k", "l", "h", "k", "l", "Angular", "Spatial", "HKL"};
    return QVariant(QString(data[section]));
  }
  return QVariant();
}

void LiveMarkerModel::highlightMarker(int n, bool b) {
  if (n<markers.size())
    markers.at(n)->highlight(b);
}

void LiveMarkerModel::deleteMarker(int n) {
  if (ZoneItem* zi=dynamic_cast<ZoneItem*>(markers.at(n))) {
    foreach(Projector* p, crystal->getConnectedProjectors()) {
      if (p->zoneMarkers().del(zi)) return;
    }
  } else if (SpotItem* si=dynamic_cast<SpotItem*>(markers.at(n))) {
    foreach(Projector* p, crystal->getConnectedProjectors()) {
      if (p->spotMarkers().del(si)) return;
    }
  }
}

void LiveMarkerModel::markerClicked() {
  AbstractMarkerItem* item = dynamic_cast<AbstractMarkerItem*>(sender());
  int idx = markers.indexOf(item);
  if (idx>=0) emit doHighlightMarker(idx);

}
