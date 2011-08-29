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

#ifndef RULERITEM_H
#define RULERITEM_H

#include "tools/propagatinggraphicsobject.h"
#include <QPen>

class CircleItem;

class RulerItem : public PropagatingGraphicsObject
{
  Q_OBJECT
public:
  RulerItem(const QPointF&, const QPointF&, double r, QGraphicsItem* parent=0);
  virtual ~RulerItem();
  QRectF boundingRect() const;
  QPainterPath shape() const;
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  void setStart(const QPointF&);
  void setEnd(const QPointF&);
  QPointF getStart();
  QPointF getEnd();
  void highlight(bool);
  bool isHighlighted();
public slots:
  void setColor(const QColor& c);
  void setHandleSize(double);
signals:
  void positionChanged();
  void itemClicked();
protected:
  CircleItem* startHandle;
  CircleItem* endHandle;
  bool highlighted;
  double radius;
private:
  RulerItem(const RulerItem& o);
  QColor rulerColor;
};

#endif // RULERITEM_H
