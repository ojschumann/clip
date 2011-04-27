/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

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
  ui->parameterView->setItemDelegateForColumn(2, new NoEditDelegate(this));

  foreach (FitObject* o, mainFitObject->getFitObjects()) {
    fitObjectAdded(o);
  }
  ui->parameterView->expandAll();

  connect(mainFitObject, SIGNAL(fitObjectAdded(FitObject*)), this, SLOT(fitObjectAdded(FitObject*)));
  connect(mainFitObject, SIGNAL(fitObjectRemoved(FitObject*)), this, SLOT(fitObjectRemoved(FitObject*)));

  fitter = new NelderMead(c, this);

  connect(ui->doFit, SIGNAL(clicked()), this, SLOT(startStopFit()));
  connect(fitter, SIGNAL(finished()), this, SLOT(toggleStartButtonText()));

  toggleStartButtonText();
}

FitDisplay::~FitDisplay()
{
  fitter->stop();
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
    ui->status->setText("Running");
    ui->status->setStyleSheet("background-color: #80C080");
    foreach (FitParameter* p, mainFitObject->allParameters()) {
      p->setValue();
    }
  } else {
    ui->doFit->setText("Start");
    ui->status->setText("idle");
    ui->status->setStyleSheet("");
  }
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
