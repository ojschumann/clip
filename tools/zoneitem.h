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

#ifndef ZONEITEM_H
#define ZONEITEM_H

#include "tools/propagatinggraphicsobject.h"
#include "tools/abstractmarkeritem.h"

#include <QPen>
#include <QDomElement>
#include "tools/vec3D.h"

class Projector;
class CircleItem;

class ZoneItem : public PropagatingGraphicsObject, public AbstractProjectorMarkerItem
{
  Q_OBJECT
public:
  ZoneItem(const QPointF&, const QPointF&, Projector* p, QGraphicsItem* _parent = nullptr);
  virtual ~ZoneItem();
  QRectF boundingRect() const;
  QPainterPath shape () const;
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  void setStart(const QPointF&);
  void setEnd(const QPointF&);
  QPointF getStart();
  QPointF getEnd();
  void highlight(bool);
  virtual Vec3D getMarkerNormal() const;

  void saveToXML(QDomElement base);
  void loadFromXML(QDomElement base);

public slots:
  void slotSetMaxSearchIndex(int);
  void colorChanged();
  void setWidth(double v);
private slots:
  void updatePolygon();
  void updateOptimalZone();
  void slotInvalidateCache();
signals:
  void positionChanged();
  void itemClicked();
protected:
  QList<QPolygonF> generatePolygon(const Vec3D&, const Vec3D&);
  QRectF imgRect;
  QRectF tightBoundingRect;
  CircleItem* startHandle;
  CircleItem* endHandle;
  bool highlighted;
  QList<QPolygonF> zoneLines;
  QList<QPolygonF> zonePolys;
  Vec3D zoneNormal;
  double markerWidth;
};

#endif // ZONEITEM_H
