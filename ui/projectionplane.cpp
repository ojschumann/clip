#include "projectionplane.h"
#include "ui_projectionplane.h"

#include <QWidget>
#include <QMouseEvent>
#include <QApplication>
#include <QGLWidget>
#include <QTimer>
#include <QSignalMapper>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QCursor>
#include <QShortcut>
#include <QPrintDialog>
#include <QSettings>

#include "ui/clip.h"
#include "ui/imagetoolbox.h"
#include "ui/resolutioncalculator.h"
#include "core/crystal.h"
#include "core/reflection.h"
#include "core/projector.h"
#include "tools/mousepositioninfo.h"
#include "tools/itemstore.h"
#include "tools/xmltools.h"
#include "image/laueimage.h"
#include "image/dataproviderfactory.h"

#include "tools/tools.h"
#include "tools/spotitem.h"
#include "tools/zoneitem.h"
#include "tools/ruleritem.h"
#include "tools/cropmarker.h"


ProjectionPlane::ProjectionPlane(Projector* p, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProjectionPlane),
    projector(p),
    projectorConfig(0),
    imageToolbox(0),
    mousePressOrigin(),
    lastMousePosition(),
    inMousePress(false),
    zoomRubber(0),
    zoomSteps()
{
  ui->setupUi(this);
  projector->setParent(this); // Ensures, that projector is deleted at end
  setWindowTitle(projector->displayName());

  connect(projector, SIGNAL(projectionRectSizeChanged()), this, SLOT(resizeView()));
  connect(projector, SIGNAL(imageLoaded(LaueImage*)), ui->view, SLOT(setImage(LaueImage*)));
  connect(projector, SIGNAL(imageLoaded(LaueImage*)), this, SLOT(imageLoaded(LaueImage*)));
  connect(projector, SIGNAL(imageClosed()), this, SLOT(imageClosed()));
  connect(projector, SIGNAL(projectorSavesDefault()), this, SLOT(saveParametersAsProjectorDefault()));
  connect(ui->view, SIGNAL(mouseMoved(QPointF)), this, SLOT(generateMousePositionInfoFromView(QPointF)));
  connect(ui->view, SIGNAL(mouseLeft()), this, SLOT(generateEmptyMousePositionInfo()));

  ui->view->setScene(projector->getScene());
  ui->view->setTransform(QTransform(1,0,0,-1,0,0));

  setupToolbar();

  contextMenuTimer.setInterval(QApplication::startDragTime());
  contextMenuTimer.setSingleShot(true);
  connect(&contextMenuTimer, SIGNAL(timeout()), this, SLOT(slotContextMenu()));

  // Call as soon as we are displayed
  QTimer::singleShot(0, this, SLOT(resizeView()));

  QSettings settings;
  settings.beginGroup(projector->projectorName());
  ui->view->setRenderHint(QPainter::Antialiasing,     settings.value("renderItem", true).toBool());
  ui->view->setRenderHint(QPainter::TextAntialiasing, settings.value("renderText", true).toBool());
  settings.endGroup();

}

ProjectionPlane::~ProjectionPlane() {
  delete ui;
}

QSize ProjectionPlane::sizeHint() const {
  return projector->projectorSizeHint();
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
  inMousePress = true;
  mousePressOrigin = ui->view->mapToScene(ui->view->viewport()->mapFromGlobal(e->globalPos()));
  if (e->buttons()==Qt::LeftButton) {
    addedDragItemOnThisMove = false;
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
    contextMenuTimer.start();
  }
  lastMousePosition = mousePressOrigin;
}

void ProjectionPlane::mouseMoveEvent(QMouseEvent *e) {
  QPointF p = ui->view->mapToScene(ui->view->viewport()->mapFromGlobal(e->globalPos()));
  QPointF dp = (p-mousePressOrigin);
  bool largeMove = fasthypot(dp.x(), dp.y())>projector->getSpotSize();
  if (e->buttons()==Qt::LeftButton) {
    if (ui->zoomAction->isChecked()) {
      zoomRubber->setGeometry(QRect(ui->view->mapFromScene(mousePressOrigin), ui->view->mapFromScene(p)).normalized());
    } else if ((ui->rulerAction->isChecked() || ui->markZonesAction->isChecked()) && largeMove) {
      if (!addedDragItemOnThisMove) {
        addedDragItemOnThisMove = true;
        if (ui->rulerAction->isChecked()) {
          projector->addRuler(mousePressOrigin, p);
        } else if (ui->markZonesAction->isChecked()) {
          projector->addZoneMarker(mousePressOrigin, p);
        }
      } else {
        if (ui->rulerAction->isChecked()) {
          projector->rulers().last()->setEnd(projector->det2img.map(p));
        } else if (ui->markZonesAction->isChecked()) {
          projector->zoneMarkers().last()->setEnd(projector->det2img.map(p));
        }
      }
      QMouseEvent e_again(QEvent::MouseButtonPress, ui->view->viewport()->mapFromGlobal(QCursor::pos()), Qt::LeftButton, e->buttons(), e->modifiers());
      inMousePress = false;
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
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    } else if (ui->rotAction->isChecked()) {
      bool b1, b2;
      Vec3D v1 = projector->det2normal(lastMousePosition, b1);
      Vec3D v2 = projector->det2normal(p, b2);
      Crystal* c=projector->getCrystal();
      if (c and b1 and b2) {
        Vec3D ax=c->getLabSystemRotationAxis();
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
        emit rotationFromProjector(180.0*M_1_PI*a);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }
  }
  generateMousePositionInfo(p);
  lastMousePosition = p;
}

void ProjectionPlane::mouseReleaseEvent(QMouseEvent *e) {
  QPointF p = ui->view->mapToScene(ui->view->viewport()->mapFromGlobal(e->globalPos()));
  QPointF dp = (p-mousePressOrigin);
  bool largeMove = fasthypot(dp.x(), dp.y()) > projector->getSpotSize();
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
        Reflection r = projector->getClosestReflection(projector->det2normal(mousePressOrigin));
        if (r.normal(0)>=0.0) {
          double TT=180.0-360.0*M_1_PI*acos(max(-1.0, min(1.0, r.normal(0))));
          QString s = r.toHtml();
          s+=QString("<br>2T=%1").arg(TT, 0,'f',1);
          projector->addInfoItem(s, mousePressOrigin);
          emit reflexInfo(r.h, r.k, r.l);
        }
      } else if (ui->markAction->isChecked()) {
        projector->addSpotMarker(p);
      }
    }
  } else if (e->button()==Qt::RightButton) {
    if (projector->spotMarkers().delAt(p)) {
    } else if (projector->zoneMarkers().delAt(p)) {
    } else if (projector->rulers().delAt(p)) {
    } else if (projector->infoItems().delAt(p)) {
    } else {
      if (zoomSteps.size()>0)
        zoomSteps.removeLast();
      resizeView();
    }
  }
  //ui->view->setDragMode(QGraphicsView::NoDrag);
  lastMousePosition = p;
  inMousePress = e->buttons() != Qt::NoButton;
  if (e->button()==Qt::RightButton)
    contextMenuTimer.stop();
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

void ProjectionPlane::on_openImgAction_triggered() {
  QString formatfilters = "Images ("+DataProviderFactory::getInstance().registeredImageFormats().replaceInStrings(QRegExp("^"), "*.").join(" ")+");;All Files (*)";
  QString fileName = QFileDialog::getOpenFileName(this,
                                                  "Load Laue pattern",
                                                  QSettings().value("LastDirectory").toString(),
                                                  formatfilters);
  QFileInfo fInfo(fileName);

  if (fInfo.exists()) {
    QSettings().setValue("LastDirectory", fInfo.canonicalFilePath());
    projector->loadImage(fileName);
  }

}

void ProjectionPlane::generateMousePositionInfoFromView(QPointF p) {
  // Rotation
  if (!inMousePress)
    generateMousePositionInfo(p);
}

void ProjectionPlane::generateMousePositionInfo(QPointF p) {
  MousePositionInfo info;
  info.valid=true;
  info.projectorPos = p;
  info.imagePos = projector->det2img.map(p);
  info.imagePos.ry() = 1.0 - info.imagePos.y();

  info.projector = projector;

  /*
   TODO: Get original pixel image position (and pixel value)
  if (projector->getLaueImage()) {
    QSizeF s = projector->getLaueImage()->transformedSize();
    info.imagePos.rx() *= s.width();
    info.imagePos.ry() *= s.height();
  }*/
  info.normal = projector->det2normal(p);
  info.detQMin = projector->Qmin();
  info.detQMax = projector->Qmax();
  info.scattered = projector->det2scattered(p, info.scatteredOk);
  if (projector->getCrystal()) {
    info.nearestOk = true;
    info.nearestReflection = projector->getClosestReflection(info.normal);
  }
  emit mousePositionInfo(info);
}

void ProjectionPlane::generateEmptyMousePositionInfo() {
  MousePositionInfo info;
  info.nearestReflection.h = 0;
  info.nearestReflection.k = 0;
  info.nearestReflection.l = 0;
  info.nearestReflection.d = 0;
  info.nearestReflection.Q = 0;
  info.nearestReflection.Qscatter = 0;
  info.nearestReflection.lowestDiffOrder = 0;
  info.nearestReflection.highestDiffOrder = 0;
  info.nearestReflection.hklSqSum = 0;
  emit mousePositionInfo(info);
}

void ProjectionPlane::on_closeImgAction_triggered() {
  projector->closeImage();
  ui->imgToolBar->setVisible(false);
  resizeView();
}

void ProjectionPlane::on_configAction_triggered() {
  if (projectorConfig.isNull()) {
    projectorConfig = projector->configWidget();
    Clip::getInstance()->addMdiWindow(projectorConfig);
  } else {
    Clip::getInstance()->setActiveSubWindow(projectorConfig);
  }
}


void ProjectionPlane::on_rotCWAction_triggered() {
  projector->doImgRotation(QTransform(0, -1, 1, 0, 0, 1));
}

void ProjectionPlane::on_rotCCWAction_triggered() {
  projector->doImgRotation(QTransform(0, 1, -1, 0, 1, 0));
}

void ProjectionPlane::on_flipHAction_triggered() {
  projector->doImgRotation(QTransform(-1, 0, 0, 1, 1, 0));
}

void ProjectionPlane::on_flipVAction_triggered() {
  projector->doImgRotation(QTransform(1, 0, 0, -1, 0, 1));
}

void ProjectionPlane::on_imageToolboxAction_triggered()
{
  if (imageToolbox.isNull()) {
    if (projector->getLaueImage()) {
      imageToolbox = new ImageToolbox(projector);
      connect(projector->getLaueImage(), SIGNAL(destroyed()), imageToolbox.data(), SLOT(deleteLater()));
      Clip::getInstance()->addMdiWindow(imageToolbox);
    }
  } else {
    Clip::getInstance()->setActiveSubWindow(imageToolbox);
  }
}

void ProjectionPlane::slotOpenResolutionCalc() {
  //on_imageToolboxAction_triggered();
}

void ProjectionPlane::on_actionCrop_triggered() {
  if (projector->getCropMarker()) {
    projector->getCropMarker()->doPublishCrop();
  } else {
    projector->showCropMarker();
  }
}

void ProjectionPlane::imageLoaded(LaueImage *img) {
  setWindowTitle(projector->displayName()+": "+img->name());
  ui->imgToolBar->setVisible(true);
  ui->rulerAction->setVisible(!projector->getLaueImage()->data()->hasData(ImageDataStore::PhysicalSize));
  resizeView();
}

void ProjectionPlane::imageClosed() {
  setWindowTitle(projector->displayName());
  ui->imgToolBar->setVisible(false);
  resizeView();
}

void ProjectionPlane::slotContextMenu() {
  QMenu context(this);

  context.addAction("Set Rotation Axis on nearest Reflection", this, SLOT(slotContextSetRotationAxisOnSpot()));
  context.addAction("Set Rotation Axis exactly here", this, SLOT(slotContextSetRotationAxis()));
  context.addAction("Clear all Markers", this, SLOT(slotContextClearAll()));
  context.addAction("Clear all Spot Markers", this, SLOT(slotContextClearSpotMarkers()));
  context.addAction("Clear all Zone Markers", this, SLOT(slotContextClearZoneMarkers()));
  context.addAction("Clear all Rulers", this, SLOT(slotContextClearRulers()));
  context.exec(QCursor::pos());

  inMousePress = false;
}

void ProjectionPlane::slotContextSetRotationAxis() {
  if (Crystal* crystal = projector->getCrystal()) {
    bool ok;
    Vec3D normal = projector->det2normal(lastMousePosition, ok);
    if (ok) crystal->setRotationAxis(normal, Crystal::LabSystem);
  }
}

void ProjectionPlane::slotContextSetRotationAxisOnSpot() {
  if (Crystal* crystal=projector->getCrystal()) {
    bool ok;
    Vec3D normal = projector->det2normal(lastMousePosition, ok);
    if (ok) crystal->setRotationAxis(projector->getClosestReflection(normal).hkl().toType<double>(), Crystal::ReziprocalSpace);
  }
}

void ProjectionPlane::slotContextClearSpotMarkers() {
  projector->spotMarkers().clear();
}

void ProjectionPlane::slotContextClearZoneMarkers() {
  projector->zoneMarkers().clear();
}

void ProjectionPlane::slotContextClearRulers() {
  projector->rulers().clear();
}

void ProjectionPlane::slotContextClearAll() {
  slotContextClearSpotMarkers();
  slotContextClearZoneMarkers();
  slotContextClearRulers();
}


void ProjectionPlane::renderPrintout(QPrinter* printer) {
  //QTextDocument
  QPainter painter(printer);
  projector->setHQPrintMode(true);
  ui->view->render(&painter);
  projector->setHQPrintMode(false);
}

#include "ui/printdialog.h"

void ProjectionPlane::on_actionPrint_triggered()
{
  PrintDialog* d = new PrintDialog(this);
  d->show();
  /*QPrintDialog printDialog(this);
  if (printDialog.exec()==QDialog::Accepted) {
    renderPrintout(printDialog.printer());

  }*/
}

const char XML_ProjectionPlane_Element[] = "ProjectionPlane";
const char XML_ProjectionPlane_type[] = "projectortype";
const char XML_ProjectionPlane_Geometry[] = "Geometry";
const char XML_ProjectionPlane_ZoomSteps[] = "ZoomSteps";
const char XML_ProjectionPlane_ZoomSteps_step[] = "Step";

void ProjectionPlane::saveToXML(QDomElement base) {
  QDomElement plane = ensureElement(base, XML_ProjectionPlane_Element);

  plane.setAttribute(XML_ProjectionPlane_type, projector->projectorName());

  if (QWidget* p = dynamic_cast<QWidget*>(parent())) {
    RectToTag(plane, XML_ProjectionPlane_Geometry, p->geometry());
  }

  QDomElement steps = plane.appendChild(plane.ownerDocument().createElement(XML_ProjectionPlane_ZoomSteps)).toElement();
  foreach (QRectF r, zoomSteps) {
    RectToTag(steps, XML_ProjectionPlane_ZoomSteps_step, r);
  }

  projector->saveToXML(plane);
}

bool ProjectionPlane::loadFromXML(QDomElement base) {
  bool ok;
  zoomSteps.clear();
  QDomElement element = base;
  if (element.tagName()!=XML_ProjectionPlane_Element)
    element = element.elementsByTagName(XML_ProjectionPlane_Element).at(0).toElement();
  if (element.isNull()) return false;
  if (element.attribute(XML_ProjectionPlane_type) != projector->projectorName()) return false;
  for (QDomElement e=element.firstChildElement(); !e.isNull(); e=e.nextSiblingElement()) {
    if (e.tagName()==XML_ProjectionPlane_Geometry) {
      if (QWidget* p = dynamic_cast<QWidget*>(parent())) {
        p->setGeometry(TagToRect(e, p->geometry()));
      }
    } else if (e.tagName()==XML_ProjectionPlane_ZoomSteps) {
      QList<QRectF> steps;
      for (QDomElement step=e.firstChildElement(); !step.isNull(); step=step.nextSiblingElement()) {
        if (step.tagName()!=XML_ProjectionPlane_ZoomSteps_step) return false;
        QRectF r = TagToRect(step, QRectF(), &ok);
        if (ok) steps << r;
      }
      zoomSteps = steps;
    }
  }
  projector->loadFromXML(element);
  return true;
}

void ProjectionPlane::saveParametersAsProjectorDefault() {
  QSettings settings;
  settings.beginGroup(projector->projectorName());
  settings.setValue("windowSize", size());
  settings.setValue("renderItem", (ui->view->renderHints() & QPainter::Antialiasing) != 0);
  settings.setValue("renderText", (ui->view->renderHints() & QPainter::TextAntialiasing) != 0);
}
