/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

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

#ifndef ROTATECRYSTAL_H
#define ROTATECRYSTAL_H

#include <QWidget>
#include <QSignalMapper>

#include "config.h"

class Crystal;

namespace Ui {
  class RotateCrystal;
}

class RotateCrystal : public QWidget
{
  Q_OBJECT

public:
  explicit RotateCrystal(QWidget* _parent = nullptr);
  virtual ~RotateCrystal();
  virtual QSize sizeHint() const;
public slots:
  void rotAxisChanged();
  void addRotationAngle(double);
  void resetSum();
private slots:
  void on_axisEdit_textEdited(QString );
  void on_axisChooser_currentIndexChanged(int index);
  void loadAxisFromCrystal();
  void addRotation(int);
  void windowChanged();

private:
  Ui::RotateCrystal *ui;
  double angleSum;
  QSignalMapper buttonMapper;
  Crystal* paramsLoadedFromThisCrystal;
  bool axisChangeFromEdit;
};

#endif // ROTATECRYSTAL_H
