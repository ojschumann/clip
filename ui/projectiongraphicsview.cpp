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

#include "projectiongraphicsview.h"
#include <QMouseEvent>
#include <QTime>
#include <QGraphicsItem>
#include <QString>
#include <iostream>

using namespace std;

ProjectionGraphicsView::ProjectionGraphicsView(QWidget *parent) :
    QGraphicsView(parent) {
  viewIgnoresThisMouseEvent=false;
}

void ProjectionGraphicsView::dragEnterEvent(QDragEnterEvent *e) {
  if (!not e->mimeData()->hasFormat("application/CrystalPointer"))
    QGraphicsView::dragEnterEvent(e);
  e->ignore();
}

void ProjectionGraphicsView::mousePressEvent(QMouseEvent *e) {
  QGraphicsView::mousePressEvent(e);
  viewIgnoresThisMouseEvent = (e->button()!=Qt::LeftButton) || !e->isAccepted();
  e->setAccepted(!viewIgnoresThisMouseEvent);
}

void ProjectionGraphicsView::mouseMoveEvent(QMouseEvent *e) {
  emit mouseMoved(mapToScene(e->pos()));
  if (viewIgnoresThisMouseEvent) {
    e->ignore();
  } else {
    QGraphicsView::mouseMoveEvent(e);
  }
}

void ProjectionGraphicsView::mouseReleaseEvent(QMouseEvent *e) {
  if (viewIgnoresThisMouseEvent) {
    e->ignore();
    viewIgnoresThisMouseEvent=false;
  } else {
    QGraphicsView::mouseReleaseEvent(e);
  }
}

void ProjectionGraphicsView::leaveEvent(QEvent *) {
  emit mouseHasLeft();
}

void ProjectionGraphicsView::setImage(LaueImage * img) {
  image = img;
}

#include <QPaintEngine>
#include <QImage>
#include <QRectF>
#include <tools/debug.h>

QPolygonF rectToPoly(const QRectF& r) {
  QPolygonF poly;
  poly << r.topLeft() << r.topRight() << r.bottomRight() << r.bottomLeft();
  return poly;
}

void ProjectionGraphicsView::drawBackground(QPainter *painter, const QRectF &rect) {
  if (image.isNull()) {
    painter->fillRect(rect, Qt::white);

  } else {
    // In the Viewport, sometimes a little more than sceneRect() is visible.
    // Use mapToScene() of the viewport dimensions to get max visible rect
    QRectF visibleSceneRectF = QRectF(mapToScene(0, 0), mapToScene(viewport()->width(), viewport()->height())).normalized();

    QTransform viewportTransform = painter->worldTransform();

    // complete visible scene Rect in device coordinates (not neccessarily viewport coordinates!)
    QRectF fullRawSceneRect = viewportTransform.mapRect(visibleSceneRectF);


    // Rect to paint on
    QRectF rawPaintRectF = viewportTransform.mapRect(rect);
    // Source rect in image is translated by offset of fullRawSceneRectF
    QRectF imgSourceRectF = rawPaintRectF.translated(-fullRawSceneRect.topLeft());

    QTransform zoomTransform;
    QTransform::quadToSquare(rectToPoly(sceneRect()), zoomTransform);
    zoomTransform = QTransform::fromScale(1, -1) * zoomTransform;

    QImage cache = image->getScaledImage(fullRawSceneRect.size().toSize(), rectToPoly(zoomTransform.mapRect(visibleSceneRectF)));

    painter->resetTransform();
    painter->drawImage(rawPaintRectF.toRect(), cache, imgSourceRectF.toRect());
    painter->setTransform(viewportTransform);
  }
}
