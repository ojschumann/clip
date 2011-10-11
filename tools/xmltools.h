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

#ifndef XMLTOOLS_H
#define XMLTOOLS_H

#include <QDomElement>
#include <QString>
#include <QRectF>
#include <QRect>

#include "config.h"

QDomDocument readXMLFile(QString filename);

template <class R> void RectToTag(QDomElement base, QString name, R rect);

QRectF TagToRect(QDomElement element, QRectF defaultValue, bool* _ok=nullptr);
QRect TagToRect(QDomElement element, QRect defaultValue, bool obeyGeometryReadSettings = true, bool* _ok=nullptr);

void PointToTag(QDomElement base, QString name, const QPointF& p);
QPointF TagToPoint(QDomElement element, const QPointF& defaultValue, bool* _ok=nullptr);

QDomElement ensureElement(QDomElement base, QString name);

int    readInt   (QDomElement element, QString name, bool& ok, int    defaultValue=0);
double readDouble(QDomElement element, QString name, bool& ok, double defaultValue=0);

#endif // XMLTOOLS_H
