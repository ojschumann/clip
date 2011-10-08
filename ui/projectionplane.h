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

#ifndef PROJECTIONPLANE_H
#define PROJECTIONPLANE_H

#include <QMainWindow>
#include <QRubberBand>
#include <QCustomEvent>
#include <QActionGroup>
#include <QPointer>
#include <QDomElement>
#include <QPrinter>
#include <QTimer>

#include "tools/objectstore.h"
#include "tools/mousepositioninfo.h"

class Projector;
class LaueImage;

namespace Ui {
  class ProjectionPlane;
}

class ProjectionPlane : public QMainWindow
{
  Q_OBJECT

public:
  explicit ProjectionPlane(Projector*, QWidget* _parent = nullptr);
  virtual ~ProjectionPlane();
  virtual void resizeEvent(QResizeEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseMoveEvent(QMouseEvent *);
  virtual void mouseReleaseEvent(QMouseEvent *);

  virtual void dragEnterEvent(QDragEnterEvent *);
  virtual void dropEvent(QDropEvent *);

  Projector* getProjector() { return projector; }
  QSize sizeHint() const;

public slots:
  void saveToXML(QDomElement base);
  bool loadFromXML(QDomElement base);

signals:
  void info(QString, int);
  void reflexInfo(int,int,int);
  void mousePositionInfo(MousePositionInfo);
  void rotationFromProjector(double);
protected slots:
  void renderPrintout(QPainter*, const QRectF&);
  void resizeView();
  void slotOpenResolutionCalc();
  void generateMousePositionInfo(QPointF, bool lock = false);
  void generateMousePositionInfoFromView(QPointF p);
  void generateEmptyMousePositionInfo();
  void slotContextMenu();
  void slotContextSetRotationAxis();
  void slotContextSetRotationAxisOnSpot();
  void slotContextLockReflectionInfoOnSpot();
  void slotContextClearSpotMarkers();
  void slotContextClearZoneMarkers();
  void slotContextClearRulers();
  void slotContextClearAll();
  void imageLoaded(LaueImage*);
  void imageClosed();
  void saveParametersAsProjectorDefault();
protected:

  void setupToolbar();
  QRectF zoomSceneRect();

  Ui::ProjectionPlane *ui;
  Projector* projector;

  QPointer<QWidget> projectorConfig;
  QPointer<QWidget> imageToolbox;

  QPointF mousePressOrigin;
  QPointF lastMousePosition;
  bool inMousePress;
  QTimer contextMenuTimer;
  // Prevents adding of multiple ZoneMarkers or Rulers. Workaround :-(
  bool addedDragItemOnThisMove;

  QRubberBand* zoomRubber;
  QList<QRectF> zoomSteps;

private slots:
  void on_actionPrint_triggered();
  void on_actionCrop_triggered();
  void on_imageToolboxAction_triggered();
  void on_flipVAction_triggered();
  void on_flipHAction_triggered();
  void on_rotCCWAction_triggered();
  void on_rotCWAction_triggered();
  void on_configAction_triggered();
  void on_openImgAction_triggered();
  void on_closeImgAction_triggered();
};




#endif // PROJECTIONPLANE_H
