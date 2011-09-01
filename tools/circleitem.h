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

#ifndef CIRCLEITEM_H
#define CIRCLEITEM_H

#include <QGraphicsObject>

class CircleItem : public QGraphicsObject
{
  Q_OBJECT
public:
  explicit CircleItem(double r, QGraphicsItem *parent = 0);
  virtual ~CircleItem();

  QRectF boundingRect() const;
  QPainterPath shape() const;
  bool contains(const QPointF &point) const;
protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant & value );
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
signals:
  void positionChanged();
  void itemClicked();
public slots:
  void setPosNoSig(const QPointF& p);
  void setRadius(double);
  void setColor(QColor);
  void setLineWidth(double);
  void doUpdate();
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  double radius;
private:
  double lineWidth;
  bool skipNextPosChange;
  QColor color;
};

#endif // CIRCLEITEM_H
