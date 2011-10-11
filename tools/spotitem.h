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

#ifndef SPOTITEM_H
#define SPOTITEM_H

#include "tools/circleitem.h"
#include "tools/abstractmarkeritem.h"

class SpotItem : public CircleItem, public AbstractProjectorMarkerItem
{
  Q_OBJECT
public:
  explicit SpotItem(Projector* p, double r, QGraphicsItem* _parent = nullptr);
  virtual Vec3D getMarkerNormal() const;
  virtual void highlight(bool b);

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  QRectF boundingRect() const;
  QPainterPath shape() const;

public slots:
  void slotSetMaxSearchIndex(int);
private slots:
  void slotInvalidateCache();
protected:
  bool isHighlighted;
};

#endif // SPOTITEM_H
