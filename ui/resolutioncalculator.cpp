#include "resolutioncalculator.h"
#include "ui_resolutioncalculator.h"

#include <core/projector.h>
#include <QAbstractTableModel>

class RulerModel: public QAbstractTableModel {
public:
  RulerModel(Projector* p, QObject* parent=0): QAbstractTableModel(parent), projector(p) {

  }

  int rowCount(const QModelIndex & parent = QModelIndex() ) const {
    return projector->rulerNumber();
  }

  int columnCount(const QModelIndex & parent = QModelIndex() ) const {
    return 4;
  }

  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const {
    if (role==Qt::DisplayRole) {
      if (index.column()==0) {
        QPair<QPointF, QPointF> c = projector->getRulerCoordinates(index.row());
        return QVariant(fabs(c.first.x()-c.second.x()));
      } else if (index.column()==1) {
        QPair<QPointF, QPointF> c = projector->getRulerCoordinates(index.row());
        return QVariant(fabs(c.first.y()-c.second.y()));
      } else if (index.column()==2) {
        QPair<QPointF, QPointF> c = projector->getRulerCoordinates(index.row());
        double dx = c.first.x()-c.second.x();
        double dy = c.first.y()-c.second.y();
        return QVariant(hypot(dx, dy));
      } else if (index.column()==3) {
        return QVariant(projector->getRulerSize(index.row()));
      }
    }
    return QVariant();
  }

  QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const {
    if (role==Qt::DisplayRole && orientation==Qt::Horizontal and section<4) {
      QStringList l;
      l << "dx" << "dy" << "Pixel Len" << "Length";
      return QVariant(l[section]);
    }
    return QVariant();
  }
  Projector* projector;
};



ResolutionCalculator::ResolutionCalculator(Projector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResolutionCalculator),
    projector(p)
{
  ui->setupUi(this);
  ui->rulerView->setModel(new RulerModel(projector));
  connect(projector, SIGNAL(destroyed()), this, SLOT(deleteLater()));
  connect(projector, SIGNAL(rulersAdded()), this, SLOT(slotReloadModel()));

}

ResolutionCalculator::~ResolutionCalculator()
{
  delete ui;
}

void ResolutionCalculator::slotReloadModel() {
  ui->rulerView->reset();
}
