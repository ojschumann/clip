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

#include "colorconfigitem.h"

#include <QSettings>

ColorConfigItem::ColorConfigItem(QString n, QColor defaultColor, QObject* _parent):
    QObject(_parent),
    _name(n)
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

