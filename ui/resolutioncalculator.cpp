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

#include "resolutioncalculator.h"
#include "ui_resolutioncalculator.h"

#include <cmath>
#include <QShortcut>
#include <QStyledItemDelegate>
#include <QAbstractTableModel>
#include <Eigen/Dense>
 

#include "tools/rulermodel.h"
#include "tools/ruleritem.h"
#include "tools/itemstore.h"
#include "tools/numberedit.h"
#include "tools/mat3D.h"
#include "tools/vec3D.h"
#include "image/laueimage.h"


class NumberEditDelegate: public QStyledItemDelegate {
public:
  NumberEditDelegate(QObject* parent=0): QStyledItemDelegate(parent) {}
  virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const {
    NumberEdit* n = new NumberEdit(parent, true);
    n->setButtonSymbols(QAbstractSpinBox::NoButtons);
    n->setMinimum(0.0);
    n->setMaximum(1000.0);
    return n;
  }
};

ResolutionCalculator::ResolutionCalculator(ItemStore<RulerItem>& r, LaueImage* img, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResolutionCalculator),
    rulers(r),
    resolutionsLocked(false),
    image(img)
{
  ui->setupUi(this);

  hRes = -1.0;
  vRes = -1.0;

  ui->rulerView->verticalHeader()->setDefaultSectionSize(ui->rulerView->fontMetrics().lineSpacing());
  ui->rulerView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  model = new RulerModel(rulers, img);
  ui->rulerView->setModel(model);
  ui->rulerView->setItemDelegate(new NumberEditDelegate);

  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotCalcResolution()));
  connect(image->data(), SIGNAL(transformChanged()), this, SLOT(slotCalcResolution()));
  connect(ui->rulerView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged()));
  connect(&r, SIGNAL(itemClicked(int)), ui->rulerView, SLOT(selectRow(int)));
  QShortcut* deleteShortcut = new QShortcut(Qt::Key_Delete, ui->rulerView);
  connect(deleteShortcut, SIGNAL(activated()), this, SLOT(deletePressed()));

  slotCalcResolution();
}

ResolutionCalculator::~ResolutionCalculator()
{
  delete ui;
  delete model;
}

QSize ResolutionCalculator::sizeHint() const {
  return minimumSizeHint();
}

void ResolutionCalculator::slotSelectionChanged() {
  QItemSelectionModel* selection = ui->rulerView->selectionModel();
  for (int n=0; n<rulers.size(); n++) {
    rulers.at(n)->highlight(selection->isRowSelected(n, QModelIndex()));
  }
}

void ResolutionCalculator::deletePressed() {
  int idx=ui->rulerView->currentIndex().row();
  rulers.del(idx);
}

struct RulerData {
  double dx;
  double dy;
  double l;
};

#ifdef _WIN32
__attribute__((force_align_arg_pointer)) void ResolutionCalculator::slotCalcResolution() {
#else
void ResolutionCalculator::slotCalcResolution() {
#endif

  // Load all rulers with a valid length in list
  QList<RulerData> rulerWithLengthData;
  QSizeF s = image->data()->getTransformedSizeData(ImageDataStore::PixelSize);
  for (int n=0; n<rulers.size(); n++) {
    bool ok;
    double l = rulers.at(n)->data(0).toDouble(&ok);
    if (ok && (l>0.0)) {
      RulerItem* r = dynamic_cast<RulerItem*>(rulers.at(n));
      RulerData d;
      d.l = l;
      d.dx = (r->getStart().x()-r->getEnd().x())*s.width();
      d.dy = (r->getStart().y()-r->getEnd().y())*s.height();
      rulerWithLengthData.append(d);
    }
  }

  int dim = resolutionsLocked ? 1 : 2;
  Eigen::Vector2d r;
  bool solutionOK = false;

  // If more amount of data is sufficient for problem, do cal
  if (rulerWithLengthData.size()>=dim) {

    // Build linear set of equations M*x =b
    Eigen::MatrixXd M(rulerWithLengthData.size(), dim);
    Eigen::VectorXd b(rulerWithLengthData.size());

    for (int n=0; n<rulerWithLengthData.size(); n++) {
      RulerData d = rulerWithLengthData.at(n);
      if (resolutionsLocked) {
        // if one resolution then (x_n^2+y_n^2)*r = l_n^2 -> M(n, 0) = x_n^2+y_n^2
        M(n, 0) = d.dx*d.dx+d.dy*d.dy;
      } else {
        // if two resolutions then r_1*x_n^2 + r2*y_n^2 = l_n^2 -> M(n,0) = x_n^2 ; M(n,1) = y_n^2
        M(n, 0) = d.dx*d.dx;
        M(n, 1) = d.dy*d.dy;
      }
      b(n) = d.l*d.l;
    }

    // Solve this system via SVD -> least square solution
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(M, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::VectorXd x = svd.solve(b);

    if (resolutionsLocked) {
      solutionOK = x(0)>0.0;
      r(0) = r(1) = x(0);
    } else {
      solutionOK = (x(0)>0.0) && (x(1)>0.0) && (svd.singularValues()(1)/svd.singularValues()(0)>1e-4);
      r=x;
    }
  }

  if (solutionOK) {
    hRes = sqrt(r(0));
    vRes = sqrt(r(1));
    ui->HResDisplay->setText(QString::number(1.0/hRes, 'f', 2));
    ui->VResDisplay->setText(QString::number(1.0/vRes, 'f', 2));
    ui->HSizeDisplay->setText(QString::number(s.width()*hRes, 'f', 2));
    ui->VSizeDisplay->setText(QString::number(s.height()*vRes, 'f', 2));
  }  else {
    ui->HResDisplay->setText("");
    ui->VResDisplay->setText("");
    ui->HSizeDisplay->setText("");
    ui->VSizeDisplay->setText("");
    hRes = -1;
    vRes = -1;
  }
  model->setResolution(hRes, vRes);
}

void ResolutionCalculator::on_acceptButton_clicked()
{
  if ((hRes>0) && (vRes>0)) {
    QSizeF s = image->data()->getTransformedSizeData(ImageDataStore::PixelSize);
    s.rwidth() *= hRes;
    s.rheight() *= vRes;
    image->data()->setTransformedSizeData(ImageDataStore::PhysicalSize, s);
  }
  rulers.clear();
}

void ResolutionCalculator::on_cancelButton_clicked()
{
  rulers.clear();
}

void ResolutionCalculator::on_pushButton_clicked()
{
  resolutionsLocked = ! resolutionsLocked;
  ui->pushButton->setIcon(QIcon(resolutionsLocked ? ":/icons/icons/lock.png" : ":/icons/icons/unlock.png"));
  slotCalcResolution();
}
