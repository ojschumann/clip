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

#include "histogramitem.h"

#include <QPainter>
#include <cmath>
#include <QTimer>
 

using namespace std;

HistogramItem::HistogramItem(QGraphicsItem *_parent) :
    QGraphicsObject(_parent)
{
  maxValue = 0;
}


QRectF HistogramItem::boundingRect() const {
  return QRectF(0,0,1,1);
}

void HistogramItem::paint(QPainter *painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
  if (maxValue == 0 || pathes.empty()) return;
  painter->save();
  QList<QColor> l;
  l << QColor(0xFF, 0x60, 0x60);
  l << QColor(0x60, 0xFF, 0x60);
  l << QColor(0x60, 0x60, 0xFF);
  l << QColor(0xFF, 0xFF, 0x60);
  l << QColor(0x60, 0xFF, 0xFF);
  l << QColor(0xFF, 0x60, 0xFF);
  l << QColor(0x7F, 0x7F, 0x7F);

  painter->setPen(Qt::NoPen);

  painter->setTransform(QTransform::fromScale(1, 1.0/maxValue), true);
  foreach (QPainterPath path, pathes) {
    painter->setBrush(QBrush(l.takeFirst()));
    painter->drawPath(path);
  }
  painter->restore();
}

void HistogramItem::setHistogram(QVector<int> r, QVector<int> g, QVector<int> b) {
  QList<QVector<int> > l;
  l << r << g << b;

  pathes.clear();
  for (int i=0; i<7; i++) {
    pathes.append(QPainterPath());
    pathes[i].moveTo(0,0);
  }


  maxValue=0;
  for (int i=0; i<r.size(); i++) {
    double xVal = 1.0*i/(r.size()-1);
    double vals[3];
    for (int n=0; n<3; n++) {
      vals[n]=l.at(n).at(i);
      if (vals[n]>0) vals[n]=log(vals[n]);
      if (maxValue<vals[n]) maxValue=vals[n];
      pathes[n].lineTo(xVal, vals[n]);
    }
    for (int n=0; n<3; n++) {
      double val = std::min(vals[n], vals[(n+1)%3]);
      pathes[n+3].lineTo(xVal, val);
    }
    pathes[6].lineTo(xVal, std::min(vals[0], std::min(vals[1], vals[2])));
  }
  for (int i=0; i<pathes.size(); i++) {
    pathes[i].lineTo(1,0);
    pathes[i].lineTo(0,0);
  }

  update();
}
