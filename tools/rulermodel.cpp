#include "rulermodel.h"

#include <core/projector.h>


RulerModel::RulerModel(Projector* p, QObject* parent): QAbstractTableModel(parent), projector(p) {
  hRes = -1.0;
  vRes = -1.0;
  connect(p, SIGNAL(rulerAdded()), this, SLOT(slotRulerAdded()));
  connect(projector, SIGNAL(rulerChanged(int)), this, SLOT(itemChanged(int)));
}

int RulerModel::rowCount(const QModelIndex & parent = QModelIndex() ) const {
  return projector->rulerNumber();
}

int RulerModel::columnCount(const QModelIndex & parent = QModelIndex() ) const {
  return 5;
}

QVariant RulerModel::data ( const QModelIndex & index, int role = Qt::DisplayRole ) const {
  if (role==Qt::DisplayRole) {
    QPair<QPointF, QPointF> c = projector->getRulerCoordinates(index.row());
    double dx = fabs(c.first.x()-c.second.x());
    double dy = fabs(c.first.y()-c.second.y());
    if (index.column()==0) {
      return QVariant(QString::number(dx, 'f', 2));
    } else if (index.column()==1) {
      QPair<QPointF, QPointF> c = projector->getRulerCoordinates(index.row());
      return QVariant(QString::number(dy, 'f', 2));
    } else if (index.column()==2) {
      return QVariant(QString::number(hypot(dx, dy), 'f', 2));
    } else if (index.column()==3) {
      QVariant v = projector->getRulerData(index.row());
      if (v.convert(QVariant::Double)) {
        return v;
      }
    } else if (index.column()==4 && hRes>0 && vRes>0) {
      return QVariant(QString::number(hypot(hRes*dx, vRes*dy), 'f', 2));
    }
  } else if (role==Qt::EditRole) {
    if (index.column()==3) {
      QVariant v = projector->getRulerData(index.row());
      v.convert(QVariant::Double);
      return v;

    }
  }
  return QVariant();
}

QVariant RulerModel::headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const {
  QStringList l;
  l << "dx" << "dy" << "Pixel Len" << "Length" << "Calc Length";
  if (role==Qt::DisplayRole && orientation==Qt::Horizontal and section<l.size()) {
    return QVariant(l[section]);
  }
  return QVariant();
}

Qt::ItemFlags RulerModel::flags(const QModelIndex &index) const {
  if (index.column()==3) {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }
  //return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return 0;
}

bool RulerModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  bool b;
  double val = value.toDouble(&b);
  if (b) {
    projector->setRulerData(index.row(), QVariant(val));
    emit dataChanged(index, index);
    return true;
  }
  if (value.toString()=="") {
    projector->setRulerData(index.row(), QVariant());
    emit dataChanged(index, index);
    return true;
  }
  return false;
}

void RulerModel::slotRulerAdded() {
  int n = projector->rulerNumber()-1;
  beginInsertRows(QModelIndex(), n, n);
  endInsertRows();
}

void RulerModel::itemChanged(int n) {
  dataChanged(index(n, 0), index(n, columnCount()-1));
}

void RulerModel::setResolution(double h, double v) {
  if (h!=hRes || v!=vRes) {
    hRes = h;
    vRes = v;
    dataChanged(index(0, 4), index(rowCount()-1, 4));
  }
}
