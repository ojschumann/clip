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

#ifndef CROPMARKER_H
#define CROPMARKER_H

#include <QSignalMapper>
#include <QGraphicsObject>

#include "config.h"

class CropMarker : public QGraphicsObject
{
  Q_OBJECT
public:
  explicit CropMarker(const QPointF& pCenter, double _dx, double _dy, double _angle, double handleSize, QGraphicsItem  *_parent = nullptr);
  virtual ~CropMarker();
  QPolygonF getRect();
  virtual void setImgTransform(const QTransform &);
  void doPublishCrop();

signals:
  void cancelCrop();
  void publishCrop(QPolygonF);
public slots:
  void promoteToRectangle() {};
  void setHandleSize(double);
  void setOutlineColor(const QColor& c);
  void setInteriorColor(const QColor& c);
protected slots:

protected:
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  QRectF boundingRect() const;
  QPainterPath shape() const;

  void mousePressEvent(QGraphicsSceneMouseEvent *);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
  void mouseMoveEvent(QGraphicsSceneMouseEvent*);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *);

  int pressedOnHandle;

  void positionHandles();
  void setCursors();

  QSizeF size;
  double handleSize;

  QList<QGraphicsRectItem*> handles;
  QColor outlineColor;
  QColor interiorColor;

  QPixmap sizeVerCursorPixmap;
  QPoint cursorHotspot;
};

#endif // CROPMARKER_H
