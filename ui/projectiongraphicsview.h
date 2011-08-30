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

#ifndef PROJECTIONGRAPHICSVIEW_H
#define PROJECTIONGRAPHICSVIEW_H

#include <QGraphicsView>
#include <image/laueimage.h>
#include <QPointer>

class ProjectionGraphicsView : public QGraphicsView
{
  Q_OBJECT
public:
  explicit ProjectionGraphicsView(QWidget *parent = 0);

  void dragEnterEvent(QDragEnterEvent *);
  void mousePressEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);
  void leaveEvent(QEvent *);
signals:
  void mouseMoved(QPointF);
  void mouseHasLeft();
public slots:
  void retakeMouseEvent() { viewIgnoresThisMouseEvent = false; }
  void setImage(LaueImage*);
protected:
  void drawBackground(QPainter *painter, const QRectF &rect);
  QPointer<LaueImage> image;

  bool viewIgnoresThisMouseEvent;

};

#endif // PROJECTIONGRAPHICSVIEW_H
