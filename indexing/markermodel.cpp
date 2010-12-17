#include "markermodel.h"

#include "core/crystal.h"
#include "core/projector.h"

MarkerModel::MarkerModel(QObject *parent) :
    QAbstractTableModel(parent)
{
  solutionValid = false;
}


void MarkerModel::setSolution(Solution s) {
  solution = s;
  solutionValid = true;
  reset();
}

void MarkerModel::clearDisplay() {
  solutionValid = false;
  reset();
}

int MarkerModel::rowCount(const QModelIndex &parent) const {
  if (solutionValid) return solution.items.size();
  return 0;
}

int MarkerModel::columnCount(const QModelIndex &parent) const {
  return 9;
}

QVariant MarkerModel::data(const QModelIndex &index, int role) const {
  if (!solutionValid) return QVariant();
  SolutionItem item = solution.items.at(index.row());
  if (role==Qt::DisplayRole) {
    int col = index.column();
    switch (col) {
    case 0:
    case 1:
    case 2:
      return QVariant(item.hkl(col));
    case 3:
    case 4:
    case 5:
      return QVariant(QString::number(item.rationalHkl(col-3), 'f', 2));
    case 6:
      return QVariant(QString::number(item.angularDeviation(), 'f', 2));
    case 7:
      return QVariant(QString::number(item.spatialDeviation(), 'f', 2));
    case 8:
      return QVariant(QString::number(item.hklDeviation(), 'f', 2));
    }
  } else if (role==Qt::BackgroundRole && item.initialIndexed) {
    return QVariant(QBrush(QColor(225, 255, 225)));
  } else if (role==Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignRight);
  }
  return QVariant();
}

QVariant MarkerModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if ((role==Qt::DisplayRole) && (orientation==Qt::Horizontal)) {
    const char* data[] = {"h", "k", "l", "h", "k", "l", "Angular", "Spatial", "HKL"};
    return QVariant(QString(data[section]));
  }
  return QVariant();
}

void MarkerModel::sort(int column, Qt::SortOrder order) {

}


