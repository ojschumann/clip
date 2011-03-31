#include "colorconfigitem.h"

#include <QSettings>

ColorConfigItem::ColorConfigItem(QString name, QColor defaultColor, QObject* parent):
    QObject(parent),
    _name(name)
{
  QSettings settings;
  setColor(settings.value(QString("colors/%1").arg(_name), defaultColor).value<QColor>());
}

ColorConfigItem::~ColorConfigItem() {
  QSettings settings;
  settings.setValue(QString("colors/%1").arg(_name), _color);
}

QColor ColorConfigItem::color() const {
  return _color;
}

QString ColorConfigItem::name() const {
  return _name;
}

void ColorConfigItem::setColor(const QColor &c) {
  _color = c;
  emit colorChanged(_color);
}

