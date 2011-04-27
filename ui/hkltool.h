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

#ifndef HKLTOOL_H
#define HKLTOOL_H

#include <QMainWindow>

namespace Ui {
  class HKLTool;
}

class Crystal;
class LiveMarkerModel;
class HKLTool : public QMainWindow
{
  Q_OBJECT

public:
  explicit HKLTool(Crystal* c, QWidget *parent = 0);
  ~HKLTool();
protected slots:
  void highlightMarkers();
  void highlightMarkerNr(int n);
  void deleteActiveMarker();
private:
  Ui::HKLTool *ui;
  LiveMarkerModel* markerModel;
};

#endif // HKLTOOL_H
