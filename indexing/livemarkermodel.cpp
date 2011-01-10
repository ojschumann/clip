#include "livemarkermodel.h"

#include "core/crystal.h"
#include "tools/spotitem.h"
#include "tools/zoneitem.h"

LiveMarkerModel::LiveMarkerModel(Crystal *c, QObject *parent) :
    QAbstractTableModel(parent),
    crystal(c)
{
  foreach (AbstractMarkerItem* item, crystal->getMarkers())
    markerAdded(item);
  connect(crystal, SIGNAL(markerAdded(AbstractMarkerItem*)), this, SLOT(markerAdded(AbstractMarkerItem*)));
  connect(crystal, SIGNAL(markerChanged(AbstractMarkerItem*)), this, SLOT(markerChanged(AbstractMarkerItem*)));
  connect(crystal, SIGNAL(markerClicked(AbstractMarkerItem*)), this, SLOT(markerClicked(AbstractMarkerItem*)));
  connect(crystal, SIGNAL(markerRemoved(AbstractMarkerItem*)), this, SLOT(markerRemoved(AbstractMarkerItem*)));
  connect(this, SIGNAL(deleteMarker(AbstractMarkerItem*)), crystal, SIGNAL(deleteMarker(AbstractMarkerItem*)));
  connect(crystal, SIGNAL(orientationChanged()), this, SLOT(rescore()));
  connect(crystal, SIGNAL(cellChanged()), this, SLOT(rescore()));
}

LiveMarkerModel::~LiveMarkerModel() {
  foreach (AbstractMarkerItem* item, markers)
    item->highlight(false);
}

void LiveMarkerModel::markerAdded(AbstractMarkerItem *m) {
  beginInsertRows(QModelIndex(), markers.size(), markers.size());
  markers << m;
  endInsertRows();
}

void LiveMarkerModel::markerChanged(AbstractMarkerItem *item) {
  int idx = markers.indexOf(item);
  emit dataChanged(index(idx, 0), index(idx, columnCount()-1));
}

void LiveMarkerModel::markerClicked(AbstractMarkerItem *item) {
  int idx = markers.indexOf(item);
  if (idx>=0) emit doHighlightMarker(idx);
}

void LiveMarkerModel::markerRemoved(AbstractMarkerItem* m) {
  int idx = markers.indexOf(m);
  beginRemoveRows(QModelIndex(), idx, idx);
  markers.removeAt(idx);
  endRemoveRows();
}

void LiveMarkerModel::highlightMarker(int n, bool b) {
  if (n<markers.size())
    markers.at(n)->highlight(b);
}


void LiveMarkerModel::deleteMarker(int n) {
  emit deleteMarker(markers.at(n));
}

void LiveMarkerModel::rescore() {
  double score = 0.0;
  foreach (AbstractMarkerItem* item, markers) {
    score += item->getIndexDeviationScore();
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
    } else if (col==7) {
      return QVariant(QString::number(markers.at(index.row())->getAngularDeviation(), 'f', 2));
    } else if (col==8) {
      return QVariant(QString::number(100.0*markers.at(index.row())->getDetectorPositionScore(), 'f', 2));
    } else if (col==9) {
      return QVariant(QString::number(100.0*markers.at(index.row())->getIndexDeviationScore(), 'f', 2));
    }
  } else if (role==Qt::BackgroundRole && false) {
    return QVariant(QBrush(QColor(225, 255, 225)));
  } else if (role==Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignRight);
  } else if (role==Qt::UserRole) { // Used for Sorting
    int col = index.column();
    if (col==0) {
      return data(index, Qt::DisplayRole);
    } else if (col==1 || col==2 || col==3) {
      TVec3D<int> n = markers.at(index.row())->getIntegerIndex();
      return QVariant(n(col-1));
    } else if (col==4 || col==5 || col==6) {
      Vec3D n = markers.at(index.row())->getRationalIndex();
      return QVariant(n(col-4));
    } else if (col==7) {
      return QVariant(markers.at(index.row())->getAngularDeviation());
    } else if (col==8) {
      return QVariant(markers.at(index.row())->getDetectorPositionScore());
    } else if (col==9) {
      return QVariant(markers.at(index.row())->getIndexDeviationScore());
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
