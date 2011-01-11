#include "fitdisplay.h"
#include "ui_fitdisplay.h"

#include <iostream>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QStyledItemDelegate>

#include "core/crystal.h"
#include "refinement/fitobject.h"
#include "refinement/fitparametergroup.h"
#include "refinement/fitparameter.h"
#include "refinement/fitparametertreeitem.h"
#include "refinement/fitparametermodel.h"

#include "refinement/neldermead.h"

using namespace std;

class NoEditDelegate: public QStyledItemDelegate {
public:
  NoEditDelegate(QObject* parent=0): QStyledItemDelegate(parent) {}
  virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return 0;
  }
};

FitDisplay::FitDisplay(Crystal* c, QWidget *parent):
    QWidget(parent),
    ui(new Ui::FitDisplay),
    mainFitObject(c)
{
  ui->setupUi(this);
  ui->parameterView->setItemDelegateForColumn(0, new NoEditDelegate(this));

  foreach (FitObject* o, mainFitObject->getFitObjects()) {
    fitObjectAdded(o);
  }

  connect(mainFitObject, SIGNAL(fitObjectAdded(FitObject*)), this, SLOT(fitObjectAdded(FitObject*)));
  connect(mainFitObject, SIGNAL(fitObjectRemoved(FitObject*)), this, SLOT(fitObjectRemoved(FitObject*)));

  fitter = new NelderMead(c, this);

  connect(ui->doFit, SIGNAL(clicked()), this, SLOT(startStopFit()));
  connect(fitter, SIGNAL(finished()), this, SLOT(toggleStartButtonText()));
  connect(fitter, SIGNAL(bestSolutionScore(double)), this, SLOT(displayScore(double)), Qt::QueuedConnection);
}

FitDisplay::~FitDisplay()
{
  delete ui;
}

void FitDisplay::startStopFit() {
  if (fitter->isRunning()) {
    fitter->stop();
  } else {
    fitter->start();
  }
  toggleStartButtonText();
}

void FitDisplay::toggleStartButtonText() {
  if (fitter->isRunning()) {
    ui->doFit->setText("Stop");
  } else {
    ui->doFit->setText("Start");
  }
}

void FitDisplay::displayScore(double score) {
  ui->scoreDisplay->setText(QString::number(100.0*score, 'f', 3));
}

void FitDisplay::fitObjectAdded(FitObject* o) {
  if (o->allParameters().size()>0) {
    QTreeWidgetItem* objectItem = new QTreeWidgetItem(ui->parameterView);
    objectItem->setText(0, o->FitObjectName());
    objectItem->setData(0, Qt::UserRole, qVariantFromValue(o));
    foreach(FitParameter* p, o->allParameters()) {
      new FitParameterTreeItem(p, objectItem);
    }
  }
}

void FitDisplay::fitObjectRemoved(FitObject* o) {
  for (int i=0; i<ui->parameterView->topLevelItemCount(); i++) {
    QVariant v = ui->parameterView->topLevelItem(i)->data(0, Qt::UserRole);
    if (o==qVariantValue<FitObject*>(v)) {
      QTreeWidgetItem* item = ui->parameterView->takeTopLevelItem(i);
      delete item;
      return;
    }
  }
}
