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
    PrimaryBeamMarker = 12
  };

  static ConfigStore* getInstance();
  static void clearInstance();

  int colorCount() const;
  QColor color(int t) const;
  QString colorName(int t) const;
  void setColor(int t, const QColor& color);
  void ensureColor(int t, const QObject* receiver, const char * method);
  void setColorChanger(int t, const QObject * sender, const char * signal);

signals:
  void colorChanged(int, QColor);
  void tmpColorChanged(QColor);

private:
  explicit ConfigStore(QObject *parent = 0);
  virtual ~ConfigStore();

  QList<ColorConfigItem*> colors;

  static ConfigStore* instance;

};

#endif // CONFIGSTORE_H
