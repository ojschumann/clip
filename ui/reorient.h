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

#ifndef REORIENT_H
#define REORIENT_H

#include <QWidget>
#include <QPair>
#include <QLineEdit>
#include <QComboBox>

#include "tools/vec3D.h"

namespace Ui {
  class Reorient;
}

class Reorient : public QWidget
{
  Q_OBJECT

public:
  explicit Reorient(QWidget *parent = 0);
  virtual ~Reorient();
  virtual QSize sizeHint() const;
public slots:
  void updateDisplay();

private:
  void gonioAxisChanged();

  Vec3D fromNormal();
  Vec3D toNormal();

  bool calcRotationAngles(double& angle1, double& angle2);
  bool calcLine(const Vec3D& nfrom, const Vec3D& nto, Vec3D& r1, Vec3D& r2);
  QList<Vec3D> calcPossibleIntermediatePositions(const Vec3D& u1, const Vec3D& u2);
  double calcRotationAngle(const Vec3D& from, const Vec3D& to, const Vec3D& axis);
  Ui::Reorient *ui;

  Vec3D fromIndex;
  Vec3D toIndex;

  QList<Vec3D> gonioAxis;
  QList<QLineEdit*> gonioEdits;
  QList<QComboBox*> gonioCombos;

private slots:
  void on_axis2Edit_textChanged(QString );
  void on_axis1Edit_textChanged(QString );
  void on_axis2Combo_currentIndexChanged(int index);
  void on_axis1Combo_currentIndexChanged(int index);
  void on_toEdit_textChanged(QString );
  void on_fromEdit_textChanged(QString );
  void on_toCombo_currentIndexChanged(int index);
  void on_doBothRotation_clicked();
  void on_doFirstRatation_clicked();

  void gonioAxisSelection(int axis, int index);
  void gonioAxisTextChanged(int axis, QString text);
  void windowChanged();

};

#endif // REORIENT_H
