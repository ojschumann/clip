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

#include "contrastcurves.h"
#include "ui_contrastcurves.h"

#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QMouseEvent>
#include <QFileDialog>

#include "image/beziercurve.h"
#include "image/laueimage.h"
#include "tools/histogramitem.h"
#include "tools/zipiterator.h"


ContrastCurves::ContrastCurves(LaueImage* img, QWidget* _parent) :
  QWidget(_parent),
  ui(new Ui::ContrastCurves),
  laueImage(img),
  handleMarkers(),
  curves()
{
  ui->setupUi(this);
  QDir dir(":/curves/curves/", "*.curve", QDir::Name|QDir::IgnoreCase, QDir::Files|QDir::Readable);
  QFileInfoList curveFiles = dir.entryInfoList();
  foreach (QFileInfo info, curveFiles) {
    QFile f (info.canonicalFilePath());
    f.open(QFile::ReadOnly);
    QDomDocument doc;
    doc.setContent(&f);
    ui->defaultCurveSelector->addItem(doc.doctype().name(), QVariant(info.canonicalFilePath()));
  }


  scene.setSceneRect(0,0,SCENEBLOWUP(1.0),SCENEBLOWUP(1.0));
  ui->gv->setScene(&scene);
  ui->gv->scale(1, -1);

  histogram = new HistogramItem();
  scene.addItem(histogram);
  connect(laueImage, SIGNAL(histogramChanged(QVector<int>,QVector<int>,QVector<int>)), histogram, SLOT(setHistogram(QVector<int>,QVector<int>,QVector<int>)));

  QPen decoPen(Qt::gray, 0, Qt::DotLine);
  foreach (double d, QList<double>() << 0.0 << SCENEBLOWUP(0.25) << SCENEBLOWUP(0.5) << SCENEBLOWUP(0.75) << SCENEBLOWUP(1.0)) {
    scene.addLine(0, d, SCENEBLOWUP(1.0), d);
    scene.addLine(d, 0, d, SCENEBLOWUP(1.0));
  }

  QList<QColor> cl = QList<QColor>() << Qt::black << Qt::red << Qt::green << Qt::blue;
  for (int n=0; n<cl.size(); n++) {
    curves << scene.addPath(QPainterPath(), QPen(cl[n], 0));
    curves.last()->setZValue(4.0-n);
    updateCurveLines(n);
  }
  activeCurve = ui->ColorSelector->currentIndex()+1;
  changeToCurve(ui->ColorSelector->currentIndex());
  connect(ui->ColorSelector, SIGNAL(activated(int)), this, SLOT(changeToCurve(int)));

}

ContrastCurves::~ContrastCurves()
{
  delete ui;
}

QSize ContrastCurves::sizeHint() const {
  return minimumSizeHint();
}

void ContrastCurves::changeToCurve(int n) {
  if (laueImage.isNull()) return;
  activeCurve=n;
  foreach (BoundedEllipse* item, handleMarkers) {
    scene.removeItem(item);
    delete item;
  }
  handleMarkers.clear();
  BezierCurve* curve = laueImage->getTransferCurves()[n];
  foreach (QPointF p, curve->getPoints()) {
    newMarker(SCENEBLOWUP(p));
  }
  handleMarkers.first()->setBBox(QRectF(0,0,0,SCENEBLOWUP(1.0)));
  handleMarkers.first()->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
  handleMarkers.last()->setBBox(QRectF(SCENEBLOWUP(1.0),0,0,SCENEBLOWUP(1.0)));
  handleMarkers.last()->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);

}

void ContrastCurves::newMarker(const QPointF& p) {
  ContrastCurves::BoundedEllipse* item = new ContrastCurves::BoundedEllipse();
  item->setPosNoSig(p);
  scene.addItem(item);
  item->setBBox(scene.sceneRect());
  item->setZValue(10);
  connect(item, SIGNAL(positionChanged()), this, SLOT(markerChanged()));
  handleMarkers << item;
  markerChanged();
}

void ContrastCurves::updateCurveLines(int n) {
  if (n<0 || laueImage.isNull() || n>=laueImage->getTransferCurves().size()) return;
  BezierCurve* curve = laueImage->getTransferCurves()[n];
  QList<QPointF> pathPoints = curve->pointRange(0.0, 0.01, 101);
  QPainterPath path(SCENEBLOWUP(pathPoints[0]));
  for (int i=1; i<pathPoints.size(); i++) {
    path.lineTo(SCENEBLOWUP(pathPoints[i]));
  }
  curves[n]->setPath(path);
}

void ContrastCurves::markerChanged() {
  if (handleMarkers.size()<2)
    return;
  QMap<double, QPointF> points;
  foreach (QGraphicsItem* item, handleMarkers)
    points.insert(SCENECOMPRESS(item->x()), SCENECOMPRESS(item->pos()));

  if (laueImage.isNull()) return;
  BezierCurve* curve = laueImage->getTransferCurves()[activeCurve];
  curve->setPoints(points.values());
  updateCurveLines(activeCurve);
  makeScales();
  ui->defaultCurveSelector->setCurrentIndex(0);
}

void ContrastCurves::mousePressEvent(QMouseEvent *e) {
  QPointF p = ui->gv->mapToScene(ui->gv->viewport()->mapFromGlobal(e->globalPos()));
  if (e->button()==Qt::LeftButton) {
    newMarker(p);
    QMouseEvent me(e->type(), ui->gv->mapFromScene(p), e->globalPos(), e->button(), e->buttons(), e->modifiers());
    ui->gv->mousePressEvent(&me);
  } else if (e->button()==Qt::RightButton) {
    BoundedEllipse* item = dynamic_cast<BoundedEllipse*>(scene.itemAt(p, ui->gv->transform()));
    if (item) {
      scene.removeItem(item);
      delete item;
      handleMarkers.removeAll(item);
      markerChanged();
    }
  }
}

void ContrastCurves::makeScales() {
  makeVScale();
  makeHScale();
}

void ContrastCurves::makeVScale() {
  QPixmap vScalePix(ui->verticalScale->size());
  QPainter p(&vScalePix);
  int w = vScalePix.width();
  int h = vScalePix.height();
  if (laueImage.isNull()) return;
  QList<BezierCurve*> bezierCurves = laueImage->getTransferCurves();
  QList<float> V = bezierCurves[0]->range(0.0, 1.0/(h-1), h);
  QList<float> R = bezierCurves[1]->map(V);
  QList<float> G = bezierCurves[2]->map(V);
  QList<float> B = bezierCurves[3]->map(V);

  for (int y=0; y<h; y++) {
    int v = int(255.0*V[y]);
    int r = int(255.0*R[y]);
    int g = int(255.0*G[y]);
    int b = int(255.0*B[y]);
    p.setPen(QColor(r,g,b));
    p.drawLine(0, h-y-1, w/2, h-y-1);

    if (ui->ColorSelector->currentIndex()==0) {
      p.setPen(QColor(v,v,v));
    } else if (ui->ColorSelector->currentIndex()==1) {
      p.setPen(QColor(r, 0, 0));
    } else if (ui->ColorSelector->currentIndex()==2) {
      p.setPen(QColor(0, g, 0));
    } else if (ui->ColorSelector->currentIndex()==3) {
      p.setPen(QColor(0, 0, b));
    }
    p.drawLine(w/2, h-y-1, w, h-y-1);

  }
  p.end();
  ui->verticalScale->setPixmap(vScalePix);

}

void ContrastCurves::makeHScale() {
  QPixmap hScalePix(ui->horizontalScale->size());
  QPainter p(&hScalePix);
  int w = hScalePix.width();
  int h = hScalePix.height();
  for (int x=0; x<w; x++) {
    int col = int(255.0*x/(w-1));
    p.setPen(QColor(col, col, col));
    p.drawLine(x, 0, x, h);
  }
  p.end();
  ui->horizontalScale->setPixmap(hScalePix);
}



ContrastCurves::BoundedEllipse::BoundedEllipse(QGraphicsItem *_parent): CircleItem(5, _parent) {
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIgnoresTransformations);
#if (QT_VERSION >= QT_VERSION_CHECK(4,6,4)) && (QT_VERSION < QT_VERSION_CHECK(4,7,0))
  // Bugfix for QGraphicsItem::ItemIgnoresTransformations and mouseMovedEvent
  // Here, a pointer to transformation infos might be nullptr and the program crashes
  // e.g. setScale initialized this pointer and everything works fine
  setScale(1.0);
#endif
  setAcceptedMouseButtons(Qt::LeftButton);
  setCursor(QCursor(Qt::SizeAllCursor));
}


bool ContrastCurves::BoundedEllipse::operator<(const BoundedEllipse& o) {
  return x()<o.x();
}

QVariant ContrastCurves::BoundedEllipse::itemChange(GraphicsItemChange change, const QVariant &value) {
  if ((change==ItemPositionChange) && !bbox.isNull()) {
    QPointF p=value.toPointF();
    if (p.x()>bbox.right()) {
      p.setX(bbox.right());
    } else if (p.x()<bbox.left()) {
      p.setX(bbox.left());
    }
    if (p.y()<bbox.top()) {
      p.setY(bbox.top());
    } else if (p.y()>bbox.bottom()) {
      p.setY(bbox.bottom());
    }
    return CircleItem::itemChange(change, QVariant(p));
  }
  return CircleItem::itemChange(change, value);
}


void ContrastCurves::loadFromFile(const QString& filename) {
  if (laueImage.isNull()) return;

  QDomDocument doc("curve");
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
    return;
  if (!doc.setContent(&file)) {
    file.close();
    return;
  }
  file.close();

  QDomElement base = doc.elementsByTagName("Curve").at(0).toElement();
  if (base.isNull()) return;
  laueImage->loadCurvesFromXML(base);
  for (int n=0; n<4; n++) updateCurveLines(n);
  changeToCurve(activeCurve);
}



void ContrastCurves::saveToFile(const QString& filename) {
  if (laueImage.isNull()) return;

  QDomDocument doc("curve");
  laueImage->saveCurvesToXML(doc.appendChild(doc.createElement("Curve")).toElement());
  QFile file(filename);

  if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
    QTextStream ts(&file);
    doc.save(ts, 1);
    file.close();
  }
}



void ContrastCurves::on_loadButton_clicked() {
  QString filename = QFileDialog::getOpenFileName(this, "Load Contrast Curve", "",
                                                  "Contrast Curves (*.curve);;All Files (*)");
  loadFromFile(filename);
}

void ContrastCurves::on_saveButton_clicked() {
  QString filename = QFileDialog::getSaveFileName(this, "Save Contrast Curve", "",
                                                  "Contrast Curves (*.curve);;All Files (*)");
  saveToFile(filename);

}

void ContrastCurves::on_defaultCurveSelector_activated(int index) {
  QString fn = ui->defaultCurveSelector->itemData(index).toString();
  if (!fn.isEmpty()) {
    loadFromFile(fn);
    // Load from file sets this to the default index
    ui->defaultCurveSelector->setCurrentIndex(index);
  }
}
