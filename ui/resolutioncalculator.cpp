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
  virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
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

__attribute__((force_align_arg_pointer)) void ResolutionCalculator::slotCalcResolution() {
  // Calc the resolution based on the minimasation of
  // S = sum_i ( ( x_i *r_x )^2 + (y_i * r_y)^2 - d_i^2 )^2
  // where r_i and r_i are the x- and y-pixelsize of a ruler
  // k_x and k_y are the searched resolutions and d_i is the
  // provided length of the ruler
  //
  // 1.) independent x- and y-resolutions
  //
  // 0 = dS/d(r_x^2) = 2*sum_i (( x_i *r_x )^2 + (y_i * r_y)^2 - d_i^2 )  x_i^2
  // 0 = dS/d(r_y^2) = 2*sum_i (( x_i *r_x )^2 + (y_i * r_y)^2 - d_i^2 )  y_i^2
  //
  // as array equation:
  // ( X  U )  (r_x) =  (v_x)
  // ( U  Y )  (r_y) =  (v_y)
  //
  // where
  // X = sum_i x_i^4
  // Y = sum_i y_i^4
  // U = sum_i x_i^2 * y_i^2
  // v_x = sum_i d_i^2 * x_i^2
  // v_y = sum_i d_i^2 * y_i^2
  //
  // 2.) single resolution r = r_x = r_y
  //
  // From array equation
  //
  // 0 = dS/d(r^2) = sum_i ( (x_i^2 + y_i^2) * r^2 - d_i^2 ) (x_i^2 + y_i^2)
  //
  // r = sum_i d_i^2 * (x_i^2 + y_i^2) / sum (x_i^2 + y_i^2)^2 = (v_x+v_y) / (X + 2*U + Y)




  Eigen::Matrix2d M;
  M = Eigen::Matrix2d::Zero();
  Eigen::Vector2d v;
  v = Eigen::Vector2d::Zero();
  QSizeF s = image->data()->getTransformedSizeData(ImageDataStore::PixelSize);
  for (int n=0; n<rulers.size(); n++) {
    bool ok;
    double l = rulers.at(n)->data(0).toDouble(&ok);
    if (ok && (l>0.0)) {
      RulerItem* r = dynamic_cast<RulerItem*>(rulers.at(n));
      double dx = (r->getStart().x()-r->getEnd().x())*s.width();
      double dy = (r->getStart().y()-r->getEnd().y())*s.height();
      M(0,0) += dx*dx*dx*dx; // X
      M(1,0) += dx*dx*dy*dy; // Y
      M(1,1) += dy*dy*dy*dy; // U
      v(0) += l*l*dx*dx;     // v_x
      v(1) += l*l*dy*dy;     // v_y
    }
  }

  Eigen::Vector2d r = Eigen::Vector2d::Zero();
  bool solutionOK = false;
  if (resolutionsLocked) {
    double denominator = (M(0,0) + 2*M(1,0) + M(1,1));
    if (denominator>1e-6) {
      r(0) = (v(0)+v(1)) / denominator;
      r(1) = r(0);
      M(0, 1) = M(1, 0);
      solutionOK = true;
    }
  } else {
    M(0, 1) = M(1, 0);
    Eigen::JacobiSVD<Eigen::Matrix2d> svd(M, Eigen::ComputeFullU | Eigen::ComputeFullV);
    double conditionNumber = svd.singularValues()(0) / svd.singularValues()(1);
    if (conditionNumber<10000.0) { // condition number is equal to the quotient of resolutions to the power of four
      r = svd.solve(v);
      solutionOK = (M*r).isApprox(v, 1e-6) && r(0)>0.0 && r(1)>0.0;
      if (false) {
        cout << "OK?" << (M*r).isApprox(v, 1e-6) << (r(0)>0.0) << (r(1)>0.0) << endl;
        cout << "ConditionNumber: " << conditionNumber << endl;
        double relativeError = (M*r-v).norm() / v.norm();
        qDebug() << "Relative Error" << relativeError;
        cout << "Matrix is: " << endl << M << endl;
        cout << "Vector is: " << endl << v << endl;
        cout << "Resolution is: " << endl << r << endl;
        cout << "M*r is: " << endl << M*r << endl << endl;
        r = M.fullPivLu().solve(v);
        cout << "Resolution is: " << endl << r << endl;
        cout << "M*r is: " << endl << M*r << endl << endl;
        for (int n=0; n<rulers.size(); n++) {
          bool ok;
          double l = rulers.at(n)->data(0).toDouble(&ok);
          if (ok && (l>0.0)) {
            RulerItem* ri = dynamic_cast<RulerItem*>(rulers.at(n));
            double dx = (ri->getStart().x()-ri->getEnd().x())*s.width();
            double dy = (ri->getStart().y()-ri->getEnd().y())*s.height();
            cout << l << " " << dx << " " << dy << " / " << l*l << " " << r(0)*dx*dx << " " << r(1)*dy*dy << endl;
          }
        }
      }
    }
    cout << "ConditionNumber " << conditionNumber << endl << M << " " <<
M.row(0).dot(M.row(1))/M.row(0).norm()/M.row(1).norm() << " " <<
M.col(0).dot(M.col(1))/M.col(0).norm()/M.col(1).norm() << " " << endl;
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
