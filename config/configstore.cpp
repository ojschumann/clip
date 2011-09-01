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

#include "configstore.h"

#include <QSettings>

ConfigStore::ConfigStore(QObject *parent) :
    QObject(parent)
{
 /* SpotMarker = 0,
    SpotMarkerHighlight = 1,
    SpotMarkerHighlightBg = 2
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
*/

  colors << new ColorConfigItem("Spot Marker",             QColor(0xFF, 0x80, 0x00), this)
      << new ColorConfigItem("Spot Marker Highlight Line", QColor(0x00, 0x00, 0x00), this)
      << new ColorConfigItem("Spot Marker Highlight BG",   QColor(0xFF, 0x33, 0x33, 0x80), this)
      << new ColorConfigItem("Zone Marker Line",           QColor(0x00, 0x00, 0x00), this)
      << new ColorConfigItem("Zone Marker Background",     QColor(0xFF, 0x80, 0x00, 0x80), this)
      << new ColorConfigItem("Zone Marker Handles",        QColor(0xFF, 0x00, 0x00), this)
      << new ColorConfigItem("Spot Indicators",            QColor(0x00, 0xFF, 0x00), this)
      << new ColorConfigItem("Spot Indicator Highlight",   QColor(0xFF, 0x00, 0x00), this)
      << new ColorConfigItem("Rulers",                     QColor(0xFF, 0x80, 0x00), this)
      << new ColorConfigItem("Rulers Handles",             QColor(0xFF, 0x00, 0x00), this)
      << new ColorConfigItem("Crop Marker Outline",        QColor(0xFF, 0x00, 0x00), this)
      << new ColorConfigItem("Crop Marker Interior",       QColor(0x80, 0x80, 0x80), this)
      << new ColorConfigItem("Primary Beam Marker",        QColor(0xFF, 0x00, 0x00), this)
      << new ColorConfigItem("HKL Indicator",              QColor(0x00, 0x00, 0x00), this);


  QSettings settings;
  zoneMarkerWidth = settings.value("ZoneMarkerWidth", 1.0).toDouble();
  loadPositionFromCWS = settings.value("LoadPositionFromWorkspace", true).toBool();
  loadSizeFromCWS = settings.value("LoadSizeFromWorkspace", true).toBool();
  initialCWSFile = settings.value("InitialWorkspaceFile", "").toString();
}

ConfigStore::~ConfigStore() {
  QSettings settings;
  settings.setValue("ZoneMarkerWidth", zoneMarkerWidth);
  settings.setValue("LoadPositionFromWorkspace", loadPositionFromCWS);
  settings.setValue("LoadSizeFromWorkspace", loadSizeFromCWS);
  settings.setValue("InitialWorkspaceFile", initialCWSFile);
}

ConfigStore* ConfigStore::instance = 0;

ConfigStore* ConfigStore::getInstance() {
  if (instance==0) {
    instance = new ConfigStore();
  }
  return instance;
}

void ConfigStore::clearInstance() {
  if (instance) {
    delete instance;
    instance = 0;
  }
}

int ConfigStore::colorCount() const {
  return colors.size();
}

QColor ConfigStore::color(int t) const {
  return colors.at(t)->color();
}

QString ConfigStore::colorName(int t) const {
  return colors.at(t)->name();
}

void ConfigStore::setColor(int t, const QColor& color) {
  colors.at(t)->setColor(color);
}

void ConfigStore::ensureColor(int t, const QObject* receiver, const char *method) {
  connect(this, SIGNAL(tmpColorChanged(QColor)), receiver, method);
  emit tmpColorChanged(color(t));
  disconnect(SIGNAL(tmpColorChanged(QColor)));
  connect(colors.at(t), SIGNAL(colorChanged(QColor)), receiver, method);
}

void ConfigStore::setColorChanger(int t, const QObject *sender, const char *signal) {
  connect(sender, signal, colors.at(t), SLOT(setColor(QColor)));
}

void ConfigStore::setZoneMarkerWidth(double v) {
  zoneMarkerWidth = v;
  emit zoneMarkerWidthChanged(v);
}

double ConfigStore::getZoneMarkerWidth() const {
  return zoneMarkerWidth;
}

void ConfigStore::setLoadPositionFromWorkspace(bool b) {
  loadPositionFromCWS = b;
}

bool ConfigStore::loadPositionFromWorkspace() {
  return loadPositionFromCWS;
}

void ConfigStore::setLoadSizeFromWorkspace(bool b) {
  loadSizeFromCWS = b;
}

bool ConfigStore::loadSizeFromWorkspace() {
  return loadSizeFromCWS;
}

void ConfigStore::setInitialWorkspaceFile(QString s) {
  initialCWSFile = s;
}

QString ConfigStore::initialWorkspaceFile() {
  return initialCWSFile;
}
