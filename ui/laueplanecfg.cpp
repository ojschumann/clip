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

#include "laueplanecfg.h"
#include "ui_laueplanecfg.h"

#include <QGraphicsView>
#include <QPair>
#include <core/laueplaneprojector.h>

LauePlaneCfg::LauePlaneCfg(LauePlaneProjector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LauePlaneCfg),
    projector(p)
{
  ui->setupUi(this);
  connect(p, SIGNAL(destroyed()), this, SLOT(deleteLater()));

  connect(ui->maxRefLabel, SIGNAL(valueChanged(int)), p, SLOT(setMaxHklSqSum(int)));
  connect(ui->setDefault, SIGNAL(clicked()), p, SLOT(saveParametersAsDefault()));
  connect(ui->detMinQ, SIGNAL(valueChanged(double)), this, SLOT(slotSetQRange()));
  connect(ui->detMaxQ, SIGNAL(valueChanged(double)), this, SLOT(slotSetQRange()));
  connect(p, SIGNAL(projectionParamsChanged()), this, SLOT(slotLoadParams()));

  connect(ui->detSpotsEnabled, SIGNAL(toggled(bool)), projector, SLOT(enableSpots(bool)));
  connect(ui->detMarkersEnabled, SIGNAL(toggled(bool)), projector, SLOT(enableMarkers(bool)));
  connect(ui->renderAntialias, SIGNAL(toggled(bool)), this, SLOT(slotUpdateRenderHints()));
  connect(ui->renderAntialiasText, SIGNAL(toggled(bool)), this, SLOT(slotUpdateRenderHints()));

  QList<QPair<NumberEdit*, QByteArray> > l;
  QPair<NumberEdit*, QByteArray> pair;

  l << qMakePair(ui->detDist, QByteArray(SLOT(setDist(double))));
  l << qMakePair(ui->detWidth, QByteArray(SLOT(setWidth(double))));
  l << qMakePair(ui->detHeight, QByteArray(SLOT(setHeight(double))));
  l << qMakePair(ui->detOmega, QByteArray(SLOT(setOmega(double))));
  l << qMakePair(ui->detChi, QByteArray(SLOT(setChi(double))));
  l << qMakePair(ui->detPhi, QByteArray(SLOT(setPhi(double))));
  l << qMakePair(ui->detDx, QByteArray(SLOT(setXOffset(double))));
  l << qMakePair(ui->detDy, QByteArray(SLOT(setYOffset(double))));

  l << qMakePair(ui->detTextSize, QByteArray(SLOT(setTextSizeFraction(double))));
  l << qMakePair(ui->detSpotSize, QByteArray(SLOT(setSpotSizeFraction(double))));

  foreach (pair, l) {
    connect(pair.first, SIGNAL(valueChanged(double)), projector, pair.second);
  }

  slotLoadParams();
}

LauePlaneCfg::~LauePlaneCfg()
{
  delete ui;
}

void LauePlaneCfg::slotSetQRange() {
  projector->setWavevectors(ui->detMinQ->value(), ui->detMaxQ->value());
  ui->QDispA->setValue(2.0*M_PI/ui->detMaxQ->value());
  ui->QDispKEV->setValue(0.5*M_1_PI*12.398*ui->detMaxQ->value());
}

void LauePlaneCfg::slotLoadParams() {
  ui->detTextSize->setValue(projector->getTextSizeFraction());
  ui->detSpotSize->setValue(projector->getSpotSizeFraction());
  ui->maxRefLabel->setValue(projector->getMaxHklSqSum());
  ui->detMinQ->setValue(projector->Qmin());
  ui->detMaxQ->setValue(projector->Qmax());

  ui->detDist->setValue(projector->dist());
  ui->detWidth->setValue(projector->width());
  ui->detHeight->setValue(projector->height());
  ui->detOmega->setValue(projector->omega());
  ui->detChi->setValue(projector->chi());
  ui->detPhi->setValue(projector->phi());
  ui->detDx->setValue(projector->xOffset());
  ui->detDy->setValue(projector->yOffset());

  ui->detSpotsEnabled->setChecked(projector->spotsEnabled());
  ui->detMarkersEnabled->setChecked(projector->markersEnabled());

  QList<QGraphicsView*> l = projector->getScene()->views();
  if (l.size()) {
    QPainter::RenderHints hints = l.at(0)->renderHints();
    ui->renderAntialias->setChecked(hints.testFlag(QPainter::Antialiasing));
    ui->renderAntialiasText->setChecked(hints.testFlag(QPainter::TextAntialiasing));
  }
}

void LauePlaneCfg::slotUpdateRenderHints() {
  foreach(QGraphicsView* view, projector->getScene()->views()) {
    view->setRenderHint(QPainter::Antialiasing, ui->renderAntialias->isChecked());
    view->setRenderHint(QPainter::TextAntialiasing, ui->renderAntialiasText->isChecked());
  }
}
