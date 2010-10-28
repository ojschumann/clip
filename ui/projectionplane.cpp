#include "projectionplane.h"
#include "ui_projectionplane.h"

#include <QMouseEvent>
#include <QApplication>
#include <QtOpenGL/QGLWidget>
#include <QTimer>
#include <QSignalMapper>
#include <QMdiArea>
#include <QMdiSubWindow>

ProjectionPlane::ProjectionPlane(Projector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionPlane)
{
  ui->setupUi(this);
  projector = p;
  projector->setParent(this); // Ensures, that projector is deleted at end
  setWindowTitle(projector->displayName());

  ui->view->setScene(projector->getScene());
  ui->view->setTransform(QTransform(1,0,0,-1,0,0));
  //ui->view->setViewport(new QGLWidget);

  //ui->toolBar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Open", this, SLOT(slotLoadCrystalData()));

  mouseModeGroup = new QActionGroup(this);
  QSignalMapper* m = new QSignalMapper(this);
  m->setMapping(mouseModeGroup->addAction(ui->toolBar->addAction(QIcon(":/zoom.png"), "Zoom")), MouseZoom);
  m->setMapping(mouseModeGroup->addAction(ui->toolBar->addAction(QIcon(":/pan.png"), "Pan")), MousePan);
  m->setMapping(mouseModeGroup->addAction(ui->toolBar->addAction(QIcon(":/rotate_left.png"), "Rotate")), MouseRotate);
  ui->toolBar->addSeparator();
  ui->toolBar->addAction(QIcon(":/configure.png"), "Configuration", this, SLOT(slotOpenProjectorConfig()));

  mouseModeGroup->setExclusive(true);
  for (int i=0; i<mouseModeGroup->actions().size(); i++) {
    mouseModeGroup->actions().at(i)->setCheckable(true);
    mouseModeGroup->actions().at(i)->setChecked(i==0);
    connect(mouseModeGroup->actions().at(i), SIGNAL(triggered()), m, SLOT(map()));
  }
  mouseMode = MouseZoom;
  connect(m, SIGNAL(mapped(int)), this, SLOT(slotChangeMouseMode(int)));

  zoomRubber=new QRubberBand(QRubberBand::Rectangle, ui->view);

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
    if (mouseMode==MouseZoom) {
      QRect r(QPoint(), QSize());
      zoomRubber->setGeometry(QRect());
      zoomRubber->show();
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
    if (mouseMode==MouseZoom) {
      zoomRubber->setGeometry(QRect(ui->view->mapFromScene(mousePressOrigin), ui->view->mapFromScene(p)).normalized());
    } else if (mouseMode==MousePan) {
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
    } else if (mouseMode==MouseRotate) {
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
    if (mouseMode==MouseZoom) {
      zoomSteps.append(QRectF(mousePressOrigin, p).normalized());
      zoomRubber->hide();
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

void ProjectionPlane::slotChangeMouseMode(int m) {
  for (int i=0; i<allPlanes.size(); i++) {
    allPlanes.at(i)->mouseMode = (MouseMode)m;
    allPlanes.at(i)->mouseModeGroup->actions().at(m)->setChecked(true);
  }
}


void ProjectionPlane::slotUpdateFPS() {
  //int frames = ui->view->getFrames();
  //ui->fpsDisplay->setText(QString::number(frames/2));
  //QTimer::singleShot(2000, this, SLOT(slotUpdateFPS()));
}

void ProjectionPlane::slotLoadCrystalData() {
}

void ProjectionPlane::slotRandomRotation() {
  projector->addRotation(Mat3D(Vec3D(1,2,3).normalized(), 0.01));
  QTimer::singleShot(1, this, SLOT(slotRandomRotation()));

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

QList<ProjectionPlane*> ProjectionPlane::allPlanes = QList<ProjectionPlane*>();
