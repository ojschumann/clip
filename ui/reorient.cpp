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

#include "reorient.h"
#include "ui_reorient.h"

#include <iostream>

#include "tools/indexparser.h"
#include "core/crystal.h"
#include "ui/clip.h"
#include "tools/tools.h"

using namespace std;

Reorient::Reorient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Reorient)
{
  ui->setupUi(this);
  gonioAxis << Vec3D() << Vec3D();
  gonioEdits << ui->axis1Edit << ui->axis2Edit;
  gonioCombos << ui->axis1Combo << ui->axis2Combo;

  gonioAxisSelection(0, ui->axis1Combo->currentIndex());
  gonioAxisSelection(1, ui->axis2Combo->currentIndex());

  connect(Clip::getInstance(), SIGNAL(windowChanged()), this, SLOT(windowChanged()));
  windowChanged();
}

Reorient::~Reorient()
{
  delete ui;
}

void Reorient::windowChanged() {
  if (Crystal* c = Clip::getInstance()->getMostRecentCrystal(true)) {
    connect(c, SIGNAL(orientationChanged()), this, SLOT(updateDisplay()), Qt::UniqueConnection);
  }
  updateDisplay();
}

QSize Reorient::sizeHint() const {
  return minimumSizeHint();
}

Vec3D Reorient::fromNormal() {
  Crystal* c;
  if (!fromIndex.isNull() && (c = Clip::getInstance()->getMostRecentCrystal(true))) {
    if (ui->RezicheckBox->isChecked()) {
      return c->hkl2Reziprocal(fromIndex).normalized();
    } else {
      return c->uvw2Real(fromIndex).normalized();
    }
  }
  return Vec3D();
}

Vec3D Reorient::toNormal() {
  int index=ui->toCombo->currentIndex();
  if (index>2) {
    Crystal* c;
    if (!toIndex.isNull() && (c = Clip::getInstance()->getMostRecentCrystal(true))) {
      if (index==3) {
        return c->hkl2Reziprocal(fromIndex).normalized();
      } else {
        return c->uvw2Real(fromIndex).normalized();
      }
    }
  } else {
    Vec3D normal;
    normal(index)=1;
    return normal;
  }
  return Vec3D();
}

void Reorient::updateDisplay() {
  double aPhi, aChi;
  if (calcRotationAngles(aChi, aPhi)) {
    ui->axis1Display->setText(QString::number(180.0*M_1_PI*aChi, 'f', 2));
    ui->axis2Display->setText(QString::number(180.0*M_1_PI*aPhi, 'f', 2));
  } else {
    ui->axis1Display->setText("-----");
    ui->axis2Display->setText("-----");
  }
}

// For the description of the algorithm, see manual.tex
bool Reorient::calcRotationAngles(double &angle1, double &angle2) {
  if (gonioAxis.at(0).isNull() || gonioAxis.at(1).isNull()) return false;
  Vec3D nfrom = fromNormal();
  if (nfrom.isNull()) return false;
  Vec3D nto = toNormal();
  if (nto.isNull()) return false;

  // Calculates a line u1+lambda*u2 in 3d-space, that is the intersection of the
  // two planes with normal Vector
  // gonioAxis[0] and gonioAxis[1] and that contain nFrom and nTo, respectively
  Vec3D u1, u2;
  if (!calcLine(nfrom, nto, u1, u2)) return false;


  double score = -1;
  // check the up to two points of intersection of the line with the unit sphere
  foreach (Vec3D v, calcPossibleIntermediatePositions(u1, u2)) {
    double aChi=calcRotationAngle(nfrom, v, gonioAxis.at(0));
    double aPhi=calcRotationAngle(v, nto, gonioAxis.at(1));
    if ((score<0) || (score > (aChi*aChi+aPhi*aPhi))) {
      angle1 = aChi;
      angle2 = aPhi;
      score = (aChi*aChi+aPhi*aPhi);
    }
  }
  if (score<0) return false;
  return true;
}


bool Reorient::calcLine(const Vec3D& nfrom, const Vec3D& nto, Vec3D& u1, Vec3D& u2) {
  u2 = gonioAxis.at(0) % gonioAxis.at(1);
  if (u2.norm()<1e-6) return false;
  u2.normalize();

  double pc = gonioAxis.at(0) * gonioAxis.at(1);
  double denom = 1.0-pc*pc;
  if (denom<1e-6) return false;

  double t1 = nfrom * gonioAxis.at(0);
  double t2 = nto   * gonioAxis.at(1);

  double lambda = (t2-pc*t1)/denom;
  double mu     = (t1-pc*t2)/denom;

  u1 = gonioAxis.at(1)*lambda + gonioAxis.at(0)*mu;

  return true;
}

QList<Vec3D> Reorient::calcPossibleIntermediatePositions(const Vec3D& u1, const Vec3D& u2) {
  QList<Vec3D> r;
  double l=u1.norm_sq();
  if (l==1.0) {
    r << u1;
  } else if (l<1.0) {
    l = sqrt(1.0-l);
    r << u1+u2*l << u1-u2*l;
  }
  return r;
}

double Reorient::calcRotationAngle(const Vec3D& from, const Vec3D& to, const Vec3D &axis) {
  Vec3D v1(from-axis*(from*axis));
  v1.normalize();

  Vec3D v2(to-axis*(to*axis));
  v2.normalize();

  double angle = acos(min(1.0, max(-1.0, v1*v2)));
  Mat3D M(from, to, axis);
  if (M.det()<0) {
    angle*=-1;
  }
  return angle;

}

void Reorient::on_doFirstRatation_clicked() {
  double aPhi, aChi;
  if (calcRotationAngles(aChi, aPhi)) {
    if (Crystal* c=Clip::getInstance()->getMostRecentCrystal(true)) {
      c->addRotation(Mat3D(gonioAxis.at(0), aChi));
    }
  }
}

void Reorient::on_doBothRotation_clicked() {
  double aPhi, aChi;
  if (calcRotationAngles(aChi, aPhi)) {
    if (Crystal* c=Clip::getInstance()->getMostRecentCrystal(true)) {
      c->addRotation(Mat3D(gonioAxis.at(0), aChi));
      c->addRotation(Mat3D(gonioAxis.at(1), aPhi));
    }
  }
}

void Reorient::on_toCombo_currentIndexChanged(int index) {
  ui->toEdit->setEnabled(index>2);
  ui->toEdit->setVisible(index>2);
  updateDisplay();
}

void Reorient::on_fromEdit_textChanged(QString text) {
  IndexParser parser(text);
  fromIndex = parser.index();
  setPaletteForStatus(ui->fromEdit, parser.isValid() && !fromIndex.isNull());
  updateDisplay();
}

void Reorient::on_toEdit_textChanged(QString text) {
  IndexParser parser(text);
  toIndex = parser.index();
  setPaletteForStatus(ui->toEdit, parser.isValid() && !toIndex.isNull());
  updateDisplay();
}

void Reorient::on_axis1Combo_currentIndexChanged(int index) {
  gonioAxisSelection(0, index);
}

void Reorient::on_axis2Combo_currentIndexChanged(int index) {
  gonioAxisSelection(1, index);
}

void Reorient::gonioAxisSelection(int axis, int index) {
  gonioEdits.at(axis)->setEnabled(index>2);
  gonioEdits.at(axis)->setVisible(index>2);
  if (index<3) {
    if (gonioCombos.at(1-axis)->currentIndex()==index)
      gonioCombos.at(1-axis)->setCurrentIndex((index+1)%3);
    gonioAxis[axis] = Vec3D();
    gonioAxis[axis](index)=1;
    updateDisplay();
  } else {
    gonioAxisTextChanged(axis, gonioEdits.at(axis)->text());
  }

}

void Reorient::on_axis1Edit_textChanged(QString text) {
  gonioAxisTextChanged(0, text);
}

void Reorient::on_axis2Edit_textChanged(QString text) {
  gonioAxisTextChanged(1, text);
}

void Reorient::gonioAxisTextChanged(int axis, QString text) {
  IndexParser parser(text);
  gonioAxis[axis] = parser.index().normalized();
  setPaletteForStatus(gonioEdits[axis], parser.isValid() && !parser.index().isNull());
  updateDisplay();
}
