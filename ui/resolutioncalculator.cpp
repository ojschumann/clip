#include "resolutioncalculator.h"
#include "ui_resolutioncalculator.h"

#include <core/projector.h>
#include <tools/rulermodel.h>
#include <QAbstractTableModel>



ResolutionCalculator::ResolutionCalculator(Projector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResolutionCalculator),
    projector(p)
{
  ui->setupUi(this);
  model = new RulerModel(projector);
  ui->rulerView->setModel(model);
  connect(projector, SIGNAL(destroyed()), this, SLOT(deleteLater()));

  ui->rulerView->verticalHeader()->setDefaultSectionSize(ui->rulerView->fontMetrics().height());
  ui->rulerView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  connect(ui->rulerView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged()));
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotCalcResolution()));

  slotCalcResolution();

}

ResolutionCalculator::~ResolutionCalculator()
{
  delete ui;
  delete model;
}

void ResolutionCalculator::slotSelectionChanged() {
  QItemSelectionModel* selection = ui->rulerView->selectionModel();
  for (int n=0; n<projector->rulerNumber(); n++) {
    bool b = selection->isRowSelected(n, QModelIndex());
    projector->highlightRuler(n, b);
  }
}

void ResolutionCalculator::slotCalcResolution() {
  double AA=0.0;
  double AB=0.0;
  double BB=0.0;
  double LA=0.0;
  double LB=0.0;

  for (int n=0; n<projector->rulerNumber(); n++) {
    QVariant v = projector->getRulerData(n);
    if (v.convert(QVariant::Double)) {
      double l = v.toDouble();
      QPair<QPointF, QPointF> c = projector->getRulerCoordinates(n);
      double dx = c.first.x()-c.second.x();
      double dy = c.first.y()-c.second.y();
      AA += dx*dx*dx*dx;
      AB += dx*dx*dy*dy;
      BB += dy*dy*dy*dy;
      LA += l*l*dx*dx;
      LB += l*l*dy*dy;
    }
  }
  Mat3D M(AA, AB, 0, AB, BB, 0, 0, 0, 1);
  if (fabs(M.det())>1e-6) {
    Vec3D v(LA, LB, 0);
    v = M.inverse()*v;
    double hRes = sqrt(fabs(v(0)));
    double vRes = sqrt(fabs(v(1)));
    ui->HResDisplay->setText(QString::number(hRes, 'f', 2));
    ui->VResDisplay->setText(QString::number(vRes, 'f', 2));
    model->setResolution(hRes, vRes);
  }  else {
    ui->HResDisplay->setText("");
    ui->VResDisplay->setText("");
    model->setResolution(-1,-1);
  }
}

void ResolutionCalculator::on_acceptButton_clicked()
{
  //TODO: Do something usefull...
  projector->clearRulers();
  deleteLater();
}

void ResolutionCalculator::on_cancelButton_clicked()
{
  projector->clearRulers();
  deleteLater();
}
