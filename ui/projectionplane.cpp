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
#include <ui/imagetoolbox.h>
#include <core/crystal.h>
#include <core/projector.h>
#include <core/reflection.h>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>

// List of all projectors. Sort of a hack ;-)
QList<ProjectionPlane*> ProjectionPlane::allPlanes = QList<ProjectionPlane*>();


ProjectionPlane::ProjectionPlane(Projector* p, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProjectionPlane),
    projector(p)
{
  ui->setupUi(this);
  projector->setParent(this); // Ensures, that projector is deleted at end
  setWindowTitle(projector->displayName());
  connect(projector, SIGNAL(projectionRectSizeChanged()), this, SLOT(resizeView()));
  connect(projector, SIGNAL(imageLoaded(LaueImage*)), ui->view, SLOT(setImage(LaueImage*)));

  ui->view->setScene(projector->getScene());
  ui->view->setTransform(QTransform(1,0,0,-1,0,0));
  //ui->view->setCacheMode(QGraphicsView::CacheBackground);
  ui->view->setCacheMode(QGraphicsView::CacheNone);

  //ui->view->viewport()->setAttribute(Qt::WA_OpaquePaintEvent, true);
  //ui->view->setBackgroundBrush(QBrush(Qt::black));
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

  // Handling for MouseDrags
  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->setExclusive(true);
  actionGroup->addAction(ui->zoomAction);
  actionGroup->addAction(ui->panAction);
  actionGroup->addAction(ui->rotAction);
  actionGroup->addAction(ui->rulerAction);
  actionGroup->addAction(ui->markZonesAction);
  ui->zoomAction->setChecked(true);

  actionGroup = new QActionGroup(this);
  actionGroup->setExclusive(true);
  actionGroup->addAction(ui->infoAction);
  actionGroup->addAction(ui->markAction);
  ui->infoAction->setChecked(true);

  ui->imgToolBar->setVisible(false);
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
  mousePressOrigin = ui->view->mapToScene(ui->view->viewport()->mapFromGlobal(e->globalPos()));
  if (e->buttons()==Qt::LeftButton) {
    if (ui->zoomAction->isChecked()) {
      if (zoomRubber) {
        delete zoomRubber;
        zoomRubber = 0;
      }
      zoomRubber = new QRubberBand(QRubberBand::Rectangle, ui->view->viewport());
      zoomRubber->setGeometry(QRect());
      zoomRubber->show();
    }
  } else if (e->buttons()==Qt::RightButton) {

    if (projector->delSpotMarkerAt(mousePressOrigin)) {
    } else if (projector->delZoneMarkerAt(mousePressOrigin)) {
    } else if (projector->delRulerAt(mousePressOrigin)) {
    } else if (projector->delInfoItemAt(mousePressOrigin)) {
    } else {
      if (zoomSteps.size()>0)
        zoomSteps.removeLast();
      resizeView();
    }
  }
  lastMousePosition = mousePressOrigin;
}

void ProjectionPlane::mouseMoveEvent(QMouseEvent *e) {
  QPointF p = ui->view->mapToScene(ui->view->viewport()->mapFromGlobal(e->globalPos()));

  cout << "Mouse pos: " << e->pos().x() << "," << e->pos().y() << " = ";
  cout << p.x() << "," << p.y() << " = ";
  QPointF q = projector->det2img.map(p);
  cout << q.x() << "," << q.y() << endl;

  QPointF dp = (p-mousePressOrigin);
  bool largeMove = hypot(dp.x(), dp.y())>0.01*projector->getSpotSize();
  if (e->buttons()==Qt::LeftButton) {
    if (ui->zoomAction->isChecked()) {
      zoomRubber->setGeometry(QRect(ui->view->mapFromScene(mousePressOrigin), ui->view->mapFromScene(p)).normalized());
    } else if (ui->rulerAction->isChecked() && largeMove) {
      projector->addRuler(mousePressOrigin, p);
      QMouseEvent e_again(QEvent::MouseButtonPress, ui->view->viewport()->mapFromGlobal(e->globalPos()), Qt::LeftButton, e->buttons(), e->modifiers());
      ui->view->mousePressEvent(&e_again);
    } else if (ui->markZonesAction->isChecked() && largeMove) {
      projector->addZoneMarker(mousePressOrigin, p);
      QMouseEvent e_again(QEvent::MouseButtonPress, ui->view->viewport()->mapFromGlobal(e->globalPos()), Qt::LeftButton, e->buttons(), e->modifiers());
      ui->view->mousePressEvent(&e_again);
    } else if (ui->panAction->isChecked()) {
      bool b1, b2;
      Vec3D v1 = projector->det2normal(lastMousePosition, b1);
      Vec3D v2 = projector->det2normal(p, b2);

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
    } else if (ui->rotAction->isChecked()) {
      bool b1, b2;
      Vec3D v1 = projector->det2normal(lastMousePosition, b1);
      Vec3D v2 = projector->det2normal(p, b2);
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
  QPointF p = ui->view->mapToScene(ui->view->viewport()->mapFromGlobal(e->globalPos()));
  QPointF dp = (p-mousePressOrigin);
  bool largeMove = hypot(dp.x(), dp.y())>0.01*projector->getSpotSize();
  if (e->button()==Qt::LeftButton) {
    if (ui->zoomAction->isChecked()) {
      if (largeMove) zoomSteps.append(QRectF(mousePressOrigin, p).normalized());
      zoomRubber->hide();
      delete zoomRubber;
      zoomRubber=0;
      resizeView();
    }
    if (!largeMove) {
      if (ui->infoAction->isChecked() && projector->getCrystal()) {
        Reflection r = projector->getCrystal()->getClosestReflection(projector->det2normal(mousePressOrigin));
        if (r.normal(0)>=0.0) {
          double TT=180.0-360.0*M_1_PI*acos(max(-1.0, min(1.0, r.normal(0))));
          QString s = r.toText();
          s+=QString("<br>2T=%1").arg(TT, 0,'f',1);
          projector->addInfoItem(s, mousePressOrigin);
          emit reflexInfo(r.h, r.k, r.l);
        }
      } else if (ui->markAction->isChecked()) {
        projector->addSpotMarker(p);
      }
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
    plane->ui->zoomAction->setChecked(ui->zoomAction->isChecked());
    plane->ui->panAction->setChecked(ui->panAction->isChecked());
    plane->ui->rotAction->setChecked(ui->rotAction->isChecked());
  }
}


void ProjectionPlane::slotUpdateFPS() {
  //int frames = ui->view->getFrames();
  //ui->fpsDisplay->setText(QString::number(frames/2));
  //QTimer::singleShot(2000, this, SLOT(slotUpdateFPS()));
}

void ProjectionPlane::slotLoadCrystalData() {
}

void ProjectionPlane::slotOpenResolutionCalc() {
  if (resoluctionCalc.isNull()) {
    resoluctionCalc = new ResolutionCalculator(projector);
    emit showConfig(resoluctionCalc);
  } else {
    QMdiSubWindow* mdi = dynamic_cast<QMdiSubWindow*>(resoluctionCalc->parent());
    if (mdi) {
      mdi->mdiArea()->setActiveSubWindow(mdi);
    }

  }
}

void ProjectionPlane::on_openImgAction_triggered() {
  QString fileName = QFileDialog::getOpenFileName(this, "Load Laue pattern", lastImageOpenDir,
                                                  "Images (*.jpg *.jpeg *.bmp *.png *.tif *.tiff *.gif *.img);;All Files (*)");
  QFileInfo fInfo(fileName);

  if (fInfo.exists()) {
    lastImageOpenDir = fInfo.canonicalFilePath();
    projector->loadImage(fileName);
    connect(ui->colorCurvesAction, SIGNAL(triggered()), projector->getLaueImage(), SLOT(showToolbox()));
  }

  ui->imgToolBar->setVisible(true);
  resizeView();
}

void ProjectionPlane::on_closeImgAction_triggered() {
  projector->closeImage();
  ui->imgToolBar->setVisible(false);
  resizeView();
}

void ProjectionPlane::on_configAction_triggered() {
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




