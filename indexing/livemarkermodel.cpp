#include "livemarkermodel.h"

#include "core/crystal.h"
#include "tools/spotitem.h"
#include "tools/zoneitem.h"

LiveMarkerModel::LiveMarkerModel(Crystal *c, QObject *parent) :
    QAbstractTableModel(parent),
    crystal(c)
{
  sortColumn = 0;
  sortOrder = Qt::AscendingOrder;
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
  QList<double> markerValues = getSortDataList();
  int idx = qLowerBound(markerValues, getSortData(m)) - markerValues.begin();
  beginInsertRows(QModelIndex(), idx, idx);
  markers.insert(idx, m);
  endInsertRows();
}

void LiveMarkerModel::markerChanged(AbstractMarkerItem *item) {
  int idx = markers.indexOf(item);
  if (idx>=0) {
    QList<double> markerValues = getSortDataList();
    // Remove changed marker Value from (otherwise ordered !) array
    double val = markerValues.takeAt(idx);
    // Check, where to insert to have the array completly ordered
    int newIdx = qLowerBound(markerValues, val) - markerValues.begin();
    // if different, move the row
    cout << "move " << idx << " -> " << newIdx << " " << val << endl;
    if (idx!=newIdx) {
      beginMoveRows(QModelIndex(), idx, idx, QModelIndex(), newIdx);
      AbstractMarkerItem* m = markers.takeAt(idx);
      markers.insert(newIdx, m);
      endMoveRows();
    }
    emit dataChanged(index(newIdx, 0), index(newIdx, columnCount()-1));
  }
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
  sort(sortColumn, sortOrder);
}

int LiveMarkerModel::rowCount(const QModelIndex &parent) const {
  return markers.size()+1;
}

int LiveMarkerModel::columnCount(const QModelIndex &parent) const {
  return 10;
}

QVariant LiveMarkerModel::data(const QModelIndex &index, int role) const {
  if (index.row()==markers.size()) {
    return sumRowData(index, role);
  } else if (role==Qt::DisplayRole) {
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
  }
  return QVariant();
}

Qt::ItemFlags LiveMarkerModel::flags(const QModelIndex &index) const {
  if (index.row()==markers.size())
    return 0;
  return QAbstractTableModel::flags(index);
}

QVariant LiveMarkerModel::sumRowData(const QModelIndex &index, int role) const {
  if (role==Qt::DisplayRole) {
    int col = index.column();
    if (col==0) {
      return QVariant(QString("Sum"));
    } else if (col>6) {
      double sum = 0.0;
      if (col==7) {
        foreach  (AbstractMarkerItem* item, markers) sum += item->getAngularDeviation();
      } else if (col==8) {
        foreach  (AbstractMarkerItem* item, markers) sum += 100.0*item->getDetectorPositionScore();
      } else if (col==9) {
        foreach  (AbstractMarkerItem* item, markers) sum += 100.0*item->getIndexDeviationScore();
      }
      return QVariant(QString::number(sum, 'f', 2));
    }
  } else if (role==Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignRight);
  } else if (role==Qt::ForegroundRole) {
    return QVariant(QBrush(Qt::black));
  } else if (role==Qt::BackgroundRole) {
    return QVariant(QBrush(QColor(0xC0, 0xC0, 0xC0)));
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

QList<double> LiveMarkerModel::getSortDataList() {
  QList<double> list;
  for (int row=0; row<markers.size(); row++) {
    list << getSortData(markers.at(row));
  }
  return list;
}

double LiveMarkerModel::getSortData(AbstractMarkerItem* m) {
  double sign = (sortOrder==Qt::AscendingOrder) ? 1.0 : -1.0;
  if (sortColumn==0) {
    return sign*(m->getType()==AbstractMarkerItem::SpotMarker)?1:2;
  } else if (sortColumn==1 || sortColumn==2 || sortColumn==3) {
    TVec3D<int> n = m->getIntegerIndex();
    return sign*n(sortColumn-1);
  } else if (sortColumn==4 || sortColumn==5 || sortColumn==6) {
    Vec3D n = m->getRationalIndex();
    return sign*n(sortColumn-4);
  } else if (sortColumn==7) {
    return sign*m->getAngularDeviation();
  } else if (sortColumn==8) {
    return sign*m->getDetectorPositionScore();
  } else if (sortColumn==9) {
    return sign*m->getIndexDeviationScore();
  }
  return 0;
}

struct SortFunctor {
  SortFunctor(double v, int i): value(v), id(i) {}
  bool operator<(const SortFunctor& o) const { return value<o.value; }
  double value;
  int id;
};

void LiveMarkerModel::sort(int column, Qt::SortOrder order) {
  sortColumn = column;
  sortOrder = order;

  QList<SortFunctor> values;
  QList<double> v = getSortDataList();
  for (int i=0; i<v.size(); i++) {
    values << SortFunctor(v.at(i), i);
  }

  qStableSort(values);

  QVector<int> ids(values.size());
  QList<AbstractMarkerItem*> tmpMarkers;
  for (int i=0; i<markers.size(); i++) {
    tmpMarkers << markers.at(values.at(i).id);
    ids[values.at(i).id]=i;
  }
  markers = tmpMarkers;
  QModelIndexList newIndices;
  foreach (QModelIndex idx, persistentIndexList()) {
    newIndices << index(ids.at(idx.row()), idx.column(), idx.parent());
  }
  changePersistentIndexList(persistentIndexList(), newIndices);
  emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}
