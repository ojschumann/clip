#include "projectionplane.h"
#include "ui_projectionplane.h"

#include <QMouseEvent>
#include <QApplication>
#include <QtOpenGL/QGLWidget>
#include <QTimer>
#include <QSignalMapper>

ProjectionPlane::ProjectionPlane(Projector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionPlane)
{
    ui->setupUi(this);
    projector = p;
    projector->setParent(this); // Ensures, that projector is deleted at end
    setWindowTitle(projector->displayName());

    ui->view->setScene(projector->getScene());
    //ui->view->setViewport(new QGLWidget);

    ui->toolBar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Open", this, SLOT(slotLoadCrystalData()));

    QActionGroup *g = new QActionGroup(this);
    g->setExclusive(true);
    QSignalMapper* m = new QSignalMapper(this);

    m->setMapping(g->addAction(ui->toolBar->addAction(QIcon(":/zoom.png"), "Zoom")), MouseZoom);
    m->setMapping(g->addAction(ui->toolBar->addAction(QIcon(":/pan.png"), "Pan")), MousePan);
    m->setMapping(g->addAction(ui->toolBar->addAction(QIcon(":/rotate_left.png"), "Rotate")), MouseRotate);

    for (int i=0; i<g->actions().size(); i++) {
        g->actions().at(i)->setCheckable(true);
        g->actions().at(i)->setChecked(i==0);
    }
    connect(m, SIGNAL(mapped(int)), this, SLOT(slotActivateMouseMode(int)));

    mouseMode = MouseZoom;

    zoomRubber=new QRubberBand(QRubberBand::Rectangle, ui->view);

    // Call as soon as we are displayed
    QTimer::singleShot(0, this, SLOT(resizeView()));
    QTimer::singleShot(2000, this, SLOT(slotUpdateFPS()));
    //QTimer::singleShot(0, this, SLOT(slotRandomRotation()));
}

ProjectionPlane::~ProjectionPlane()
{
    delete ui;
    delete zoomRubber;
    delete projector;
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

    //e->source()->
    //c=e.source().crystal
    //self.projector.connectToCrystal(c)
}

void ProjectionPlane::slotActivateMouseMode(int mode) {
  mouseMode = MouseMode(mode);
  emit mouseModeChanged(mode);
}

void ProjectionPlane::slotUpdateFPS() {
  int frames = ui->view->getFrames();
  ui->fpsDisplay->setText(QString::number(frames/2));
  QTimer::singleShot(2000, this, SLOT(slotUpdateFPS()));
}

void ProjectionPlane::slotLoadCrystalData() {
}

void ProjectionPlane::slotRandomRotation() {
  projector->addRotation(Mat3D(Vec3D(1,2,3).normalized(), 0.01));
  QTimer::singleShot(1, this, SLOT(slotRandomRotation()));

}
