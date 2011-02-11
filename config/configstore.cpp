#include "configstore.h"

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
      << new ColorConfigItem("Primary Beam Marker",        QColor(0xFF, 0x00, 0x00), this);

}

ConfigStore::~ConfigStore() {

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
