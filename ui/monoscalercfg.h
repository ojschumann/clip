/**************************************************************************
  Copyright (C) 2011 schumann

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
**************************************************************************/

#ifndef MONOSCALERCFG_H
#define MONOSCALERCFG_H

#include <QWidget>

namespace Ui {
  class MonoScalerCfg;
}

class MonoScalerCfg : public QWidget
{
  Q_OBJECT

public:
  explicit MonoScalerCfg(bool histEq, bool logMap, QWidget *parent = 0);
  ~MonoScalerCfg();
signals:
  void histogramEq(bool);
  void logMapping(bool);
private:
  Ui::MonoScalerCfg *ui;
};

#endif // MONOSCALERCFG_H
