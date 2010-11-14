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

  ui->rulerView->verticalHeader()->setDefaultSectionSize(ui->rulerView->fontMetrics().height());
  ui->rulerView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  model = new RulerModel(projector);
  ui->rulerView->setModel(model);
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotCalcResolution()));
  connect(ui->rulerView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged()));

  slotCalcResolution();

}

ResolutionCalculator::~ResolutionCalculator()
{
  delete ui;
  delete model;
}

void ResolutionCalculator::slotSelectionChanged() {
  if (projector.isNull()) return;
  QItemSelectionModel* selection = ui->rulerView->selectionModel();
  for (int n=0; n<projector->rulerNumber(); n++) {
    bool b = selection->isRowSelected(n, QModelIndex());
    projector->highlightRuler(n, b);
  }
}

void ResolutionCalculator::slotCalcResolution() {
  if (projector.isNull()) return;
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
  if (projector.isNull()) return;
  //TODO: Do something usefull...
  projector->clearRulers();
}

void ResolutionCalculator::on_cancelButton_clicked()
{
  if (projector.isNull()) return;
  projector->clearRulers();
}
