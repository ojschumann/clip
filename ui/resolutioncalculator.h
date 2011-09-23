/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

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

#ifndef RESOLUTIONCALCULATOR_H
#define RESOLUTIONCALCULATOR_H

#include <QWidget>
#include "tools/itemstore.h"

namespace Ui {
  class ResolutionCalculator;
}

class RulerModel;
class RulerItem;
class LaueImage;

class ResolutionCalculator : public QWidget
{
  Q_OBJECT

public:
  explicit ResolutionCalculator(ItemStore<RulerItem>& r, LaueImage* img, QWidget* _parent = nullptr);
  virtual ~ResolutionCalculator();
  virtual QSize sizeHint() const;
public slots:
  void slotSelectionChanged();
  void slotCalcResolution();
protected slots:
  void deletePressed();
private:
  Ui::ResolutionCalculator *ui;
  ItemStore<RulerItem>& rulers;
  RulerModel* model;
  bool resolutionsLocked;
  LaueImage* image;
  double hRes;
  double vRes;
private slots:
    void on_pushButton_clicked();
    void on_cancelButton_clicked();
    void on_acceptButton_clicked();

};

#endif // RESOLUTIONCALCULATOR_H
