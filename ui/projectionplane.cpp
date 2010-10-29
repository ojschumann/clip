#include "projectionplane.h"
#include "ui_projectionplane.h"

#include <QMouseEvent>
#include <QApplication>
#include <QtOpenGL/QGLWidget>
#include <QTimer>
#include <QSignalMapper>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <ui/resolutioncalculator.h>

ProjectionPlane::ProjectionPlane(Projector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionPlane),
    projector(p)
{
  ui->setupUi(this);
  projector->setParent(this); // Ensures, that projector is deleted at end
  setWindowTitle(projector->displayName());

  ui->view->setScene(projector->getScene());
  ui->view->setTransform(QTransform(1,0,0,-1,0,0));
  //ui->view->setViewport(new QGLWidget);

  setupToolbar();

  zoomRubber=0;

  // Call as soon as we are displayed
  QTimer::singleShot(0, this, SLOT(resizeView()));
  QTimer::singleShot(2000, this, SLOT(slotUpdateFPS()));
  //QTimer::singleShot(0, this, SLOT(slotRandomRotation()));
  allPlanes << this;
}

ProjectionPlane::~ProjectionPlane() {
  allPlanes.removeAll(this);
  delete ui;
}

void ProjectionPlane::setupToolbar() {
  zoomAction = ui->toolBar->addAction(QIcon(":/zoom.png"), "Zoom", this, SLOT(slotChangeMouseDragMode()));
  panAction = ui->toolBar->addAction(QIcon(":/pan.png"), "Pan", this, SLOT(slotChangeMouseDragMode()));
  rotAction = ui->toolBar->addAction(QIcon(":/rotate_cw.png"), "Rotate", this, SLOT(slotChangeMouseDragMode()));
  rulerAction = ui->toolBar->addAction(QIcon(":/info.png"), "Measure resolution", this, SLOT(slotChangeMouseDragMode()));
  connect(rulerAction, SIGNAL(triggered()), this, SLOT(slotOpenResolutionCalc()));

  ui->toolBar->addSeparator();

  markAction = ui->toolBar->addAction(QIcon(":/flag.png"), "Mark Spots");
  infoAction = ui->toolBar->addAction(QIcon(":/info.png"), "Info on Spot");

  ui->toolBar->addSeparator();

  openImgAction = ui->toolBar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Open Image", this, SLOT(slotLoadImage()));
  closeImgAction = ui->toolBar->addAction(style()->standardIcon(QStyle::SP_DialogCloseButton), "Close Image", this, SLOT(slotCloseImage()));

  flipHAction = ui->toolBar->addAction(QIcon(":/flip_horizontal.png"), "Flip horizontally");
  flipVAction = ui->toolBar->addAction(QIcon(":/flip_vertical.png"), "Flip horizontally");
  rotCWAction = ui->toolBar->addAction(QIcon(":/rotate_cw.png"), "Rotate Image clockwise");
  rotCCWAction = ui->toolBar->addAction(QIcon(":/rotate_ccw.png"), "Rotate Image counterclockwise");
  colorCurveAction = ui->toolBar->addAction(QIcon(":/rotate_ccw.png"), "Open Color Curve Tool");

  ui->toolBar->addSeparator();

  printAction = ui->toolBar->addAction(QIcon(":/fileprint.png"), "Flip horizontally");
  configAction = ui->toolBar->addAction(QIcon(":/configure.png"), "Configuration", this, SLOT(slotOpenProjectorConfig()));

  // Handling for MouseDrags
  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->setExclusive(true);
  actionGroup->addAction(zoomAction);
  actionGroup->addAction(panAction);
  actionGroup->addAction(rotAction);
  actionGroup->addAction(rulerAction);
  foreach (QAction* action, actionGroup->actions()) {
    action->setCheckable(true);
  }
  zoomAction->setChecked(true);

  actionGroup = new QActionGroup(this);
  actionGroup->setExclusive(true);
  actionGroup->addAction(infoAction);
  actionGroup->addAction(markAction);
  foreach (QAction* action, actionGroup->actions()) {
    action->setCheckable(true);
  }
  infoAction->setChecked(true);

  imageTools << rulerAction << closeImgAction << flipHAction << flipVAction << rotCWAction << rotCCWAction << colorCurveAction;
  foreach (QAction* action, imageTools)
    action->setVisible(false);
  /*zoomAction->setChecked(true);
  connect(mouseDragModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotChangeMouseDragMode(QAction*)));
  */

  // Hide if no Image
/*  imageToolsGroup = new QActionGroup(this);
  imageToolsGroup->setExclusive(false);
  imageToolsGroup->setVisible(false);
  //imageToolsGroup->addAction(rulerAction);
  imageToolsGroup->addAction(closeImgAction);
  imageToolsGroup->addAction(flipHAction);
  imageToolsGroup->addAction(flipVAction);
  imageToolsGroup->addAction(rotCWAction);
  imageToolsGroup->addAction(rotCCWAction); */
}


QRectF ProjectionPlane::zoomSceneRect() {
  if (!zoomSteps.empty()) {
    return zoomSteps.last();
  }
  return projector->getScene()->sceneRect();
}

void ProjectionPlane::resizeView() {
  // Get ZoomRect
  QRectF minViewRect = zoomSceneRect();
  // Get its Size
  QSizeF minWidgetSize = minViewRect.size();
  // Scale that Size to the container size, keeping its AspectRatio
  minWidgetSize.scale(ui->viewFrame->size(), Qt::KeepAspectRatio);
  // It was scaled by that factor
  double scaleFactor = minWidgetSize.width()/minViewRect.width();
  // Scale the full Scene by that factor
  QSizeF maxWidgetSize = scaleFactor * projector->getScene()->sceneRect().size();
  // bound that to the widget and use as rectSize
  QRectF finalRect(QPointF(0,0), maxWidgetSize.boundedTo(ui->viewFrame->size()));
  // Center the View
  finalRect.moveCenter(ui->viewFrame->rect().center());
  // And set the Geometry
  ui->view->setGeometry(finalRect.toRect());
  // Set the ZoomRect to the view
  ui->view->fitInView(minViewRect, Qt::KeepAspectRatio);
}

void ProjectionPlane::resizeEvent(QResizeEvent *e) {
  resizeView();
}


void ProjectionPlane::mousePressEvent(QMouseEvent *e) {
  mousePressOrigin = ui->view->mapToScene(ui->view->mapFromGlobal(e->globalPos()));
  if (e->buttons()==Qt::LeftButton) {
    if (zoomAction->isChecked()) {
      if (zoomRubber) {
        delete zoomRubber;
      }
      zoomRubber = new QRubberBand(QRubberBand::Rectangle, ui->view);
      zoomRubber->setGeometry(QRect());
      zoomRubber->show();
    } else if (rulerAction->isChecked()) {
      projector->addRuler(mousePressOrigin, mousePressOrigin);
    }
  } else if (e->buttons()==Qt::RightButton) {
    if (zoomSteps.size()>0)
      zoomSteps.removeLast();
    resizeView();
  }
  lastMousePosition = mousePressOrigin;
}

void ProjectionPlane::mouseMoveEvent(QMouseEvent *e) {
  QPointF p = ui->view->mapToScene(ui->view->mapFromGlobal(e->globalPos()));
  if (e->buttons()==Qt::LeftButton) {
    if (zoomAction->isChecked()) {
      zoomRubber->setGeometry(QRect(ui->view->mapFromScene(mousePressOrigin), ui->view->mapFromScene(p)).normalized());
    } else if (rulerAction->isChecked()) {
      projector->updateMostRecentRuler(p);
    } else if (panAction->isChecked()) {
      bool b1, b2;
      Vec3D v1 = projector->det2normal(lastMousePosition, &b1);
      Vec3D v2 = projector->det2normal(p, &b2);

      if (b1 and b2) {
        Vec3D r=v1%v2;
        r.normalize();
        projector->addRotation(r, acos(v1*v2));

        // Process screen updates. Otherwise on Windows no updates are prosessed if
        // two projectors are active and the mouse moves fast.
        // Otherwise, on Linux this produces stange effects.
        //if self.doProcessEvent:
        //    QtGui.qApp.processEvents(QtCore.QEventLoop.ExcludeUserInputEvents)
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    } else if (rotAction->isChecked()) {
      bool b1, b2;
      Vec3D v1 = projector->det2normal(lastMousePosition, &b1);
      Vec3D v2 = projector->det2normal(p, &b2);
      Crystal* c=projector->getCrystal();
      if (c and b1 and b2) {
        Vec3D ax=c->getLabSystamRotationAxis();
        v1=v1-ax*(v1*ax);
        v2=v2-ax*(v2*ax);
        v1.normalize();
        v2.normalize();
        double a=v1*v2;
        if (a>1.0) a=1.0;
        a=acos(a);
        if (Mat3D(ax, v1, v2).det()<0)
          a*=-1;
        projector->addRotation(ax, a);
        //self.emit(QtCore.SIGNAL('projectorAddedRotation(double)'), a)
        //if self.doProcessEvent:
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }
  }
  lastMousePosition = p;
}

void ProjectionPlane::mouseReleaseEvent(QMouseEvent *e) {
  QPointF p = ui->view->mapToScene(ui->view->mapFromGlobal(e->globalPos()));
  if (e->button()==Qt::LeftButton) {
    if (zoomAction->isChecked()) {
      zoomSteps.append(QRectF(mousePressOrigin, p).normalized());
      zoomRubber->hide();
      delete zoomRubber;
      zoomRubber=0;
      resizeView();
    }
  }
  //ui->view->setDragMode(QGraphicsView::NoDrag);
  lastMousePosition = p;
}


void ProjectionPlane::dragEnterEvent(QDragEnterEvent *e) {
  if (e->mimeData()->hasFormat("application/CrystalPointer"))
    e->acceptProposedAction();
}

void ProjectionPlane::dropEvent(QDropEvent *e) {
  e->acceptProposedAction();
  if (e->mimeData()->hasImage()) {
    QVariant v = e->mimeData()->imageData();
    if (v.canConvert<Crystal*>()) {
      Crystal* c = v.value<Crystal*>();
      projector->connectToCrystal(c);
    }
  }
}

void ProjectionPlane::slotChangeMouseDragMode() {
  foreach(ProjectionPlane* plane, allPlanes) {
    plane->zoomAction->setChecked(zoomAction->isChecked());
    plane->panAction->setChecked(panAction->isChecked());
    plane->rotAction->setChecked(rotAction->isChecked());
  }
}


void ProjectionPlane::slotUpdateFPS() {
  //int frames = ui->view->getFrames();
  //ui->fpsDisplay->setText(QString::number(frames/2));
  //QTimer::singleShot(2000, this, SLOT(slotUpdateFPS()));
}

void ProjectionPlane::slotLoadCrystalData() {
}

void ProjectionPlane::slotLoadImage() {
  foreach (QAction* action, imageTools)
    action->setVisible(true);

}

void ProjectionPlane::slotCloseImage() {
  foreach (QAction* action, imageTools)
    action->setVisible(false);

}


void ProjectionPlane::slotOpenProjectorConfig() {
  if (projectorConfig.isNull()) {
    projectorConfig = projector->configWidget();
    emit showConfig(projectorConfig);
  } else {
    QMdiSubWindow* mdi = dynamic_cast<QMdiSubWindow*>(projectorConfig->parent());
    if (mdi) {
      mdi->mdiArea()->setActiveSubWindow(mdi);
    }

  }
}

void ProjectionPlane::slotOpenResolutionCalc() {
  if (resoluctionCalc.isNull()) {
    resoluctionCalc = new ResolutionCalculator(projector);
    emit showConfig(projectorConfig);
  } else {
    QMdiSubWindow* mdi = dynamic_cast<QMdiSubWindow*>(projectorConfig->parent());
    if (mdi) {
      mdi->mdiArea()->setActiveSubWindow(mdi);
    }

  }
}


QList<ProjectionPlane*> ProjectionPlane::allPlanes = QList<ProjectionPlane*>();
