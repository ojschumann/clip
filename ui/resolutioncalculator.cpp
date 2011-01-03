#include "resolutioncalculator.h"
#include "ui_resolutioncalculator.h"

#include <cmath>

#include <tools/rulermodel.h>
#include <QAbstractTableModel>
#include "tools/ruleritem.h"
#include "tools/itemstore.h"
#include "tools/mat3D.h"
#include "tools/vec3D.h"

ResolutionCalculator::ResolutionCalculator(ItemStore<RulerItem>& r, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResolutionCalculator),
    rulers(r)
{
  ui->setupUi(this);

  ui->rulerView->verticalHeader()->setDefaultSectionSize(ui->rulerView->fontMetrics().height());
  ui->rulerView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  model = new RulerModel(rulers);
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
  QItemSelectionModel* selection = ui->rulerView->selectionModel();
  for (int n=0; n<rulers.size(); n++) {
    bool b = selection->isRowSelected(n, QModelIndex());
    // Todo: make better
    dynamic_cast<RulerItem*>(rulers.at(n))->highlight(b);
  }
}

void ResolutionCalculator::slotCalcResolution() {
  double AA=0.0;
  double AB=0.0;
  double BB=0.0;
  double LA=0.0;
  double LB=0.0;

  for (int n=0; n<rulers.size(); n++) {
    QVariant v = rulers.at(n)->data(0);
    if (v.convert(QVariant::Double)) {
      double l = v.toDouble();
      RulerItem* r = dynamic_cast<RulerItem*>(rulers.at(n));
      double dx = r->getStart().x()-r->getEnd().x();
      double dy = r->getStart().y()-r->getEnd().y();
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
  rulers.clear();
}

void ResolutionCalculator::on_cancelButton_clicked()
{
  rulers.clear();
}
