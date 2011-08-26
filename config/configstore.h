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

#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QSignalMapper>

#include "config/colorconfigitem.h"

class ConfigStore : public QObject
{
  Q_OBJECT
public:
  enum ColorType {
    SpotMarker = 0,
    SpotMarkerHighlight = 1,
    SpotMarkerHighlightBg = 2,
    ZoneMarkerLine = 3,
    ZoneMarkerBackground = 4,
    ZoneMarkerHandles = 5,
    SpotIndicators = 6,
    SpotIndicatorHighlight = 7,
    Ruler = 8,
    RulerHandles = 9,
    CropMarkerOutline = 10,
    CropMarkerInterior = 11,
    PrimaryBeamMarker = 12,
    HKLIndicator = 13
  };

  static ConfigStore* getInstance();
  static void clearInstance();

  int colorCount() const;
  QColor color(int t) const;
  QString colorName(int t) const;
  void setColor(int t, const QColor& color);
  void ensureColor(int t, const QObject* receiver, const char * method);
  void setColorChanger(int t, const QObject * sender, const char * signal);
  double getZoneMarkerWidth() const;
  bool loadPositionFromWorkspace();
  bool loadSizeFromWorkspace();
  QString initialWorkspaceFile();
public slots:
  void setZoneMarkerWidth(double);
  void setLoadPositionFromWorkspace(bool);
  void setLoadSizeFromWorkspace(bool);
  void setInitialWorkspaceFile(QString);
signals:
  void colorChanged(int, QColor);
  void zoneMarkerWidthChanged(double);
  void tmpColorChanged(QColor);

private:
  explicit ConfigStore(QObject *parent = 0);
  virtual ~ConfigStore();

  QList<ColorConfigItem*> colors;
  double zoneMarkerWidth;
  bool loadPositionFromCWS;
  bool loadSizeFromCWS;
  static ConfigStore* instance;
  QString initialCWSFile;

};

#endif // CONFIGSTORE_H
