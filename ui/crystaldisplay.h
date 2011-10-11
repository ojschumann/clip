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

#ifndef CRYSTALDISPLAY_H
#define CRYSTALDISPLAY_H

#include <QMainWindow>
#include <QPointer>
#include <QDomElement>

#include "ui/hkltool.h"

class Crystal;

namespace Ui {
  class CrystalDisplay;
}

class CrystalDisplay : public QMainWindow
{
  Q_OBJECT

public:
  explicit CrystalDisplay(QWidget* _parent = nullptr);
  virtual ~CrystalDisplay();

  void loadFromXML(QDomElement base);
  void saveToXML(QDomElement base);
  void loadDefault();

  QSize sizeHint() const;

  Crystal* getCrystal() { return crystal; }

  virtual void dragEnterEvent(QDragEnterEvent *);
  virtual void dropEvent(QDropEvent *);

signals:
  void info(QString, int);
public slots:
  void slotUpdateOrientationMatrix();
  void slotCellChanged();
  void slotRotationChanged();
  void slotSpacegroupChanged(QString);
  void slotLoadCellFromCrystal();
  void slotStartIndexing();
  void slotSetSGConstrains();

private:
  Ui::CrystalDisplay *ui;
  Crystal* crystal;
  bool allowRotationUpdate;
  QPointer<HKLTool> hklTool;

private slots:
  void on_actionSave_triggered();
  void on_actionLoad_triggered();
  void on_actionDrag_hovered();
};

#endif // CRYSTAL_H
