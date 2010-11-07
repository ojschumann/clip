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
  frames=0;
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
  //cout << "Mouse pos: " << e->posF().x() << "," << e->posF().y() << " = ";
  //QPointF p = mapToScene(e->pos());
  //cout << p.x() << "," << p.y() << endl;
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

int ProjectionGraphicsView::getFrames() {
  int f = frames;
  frames = 0;
  return f;
}

void ProjectionGraphicsView::paintEvent(QPaintEvent *e) {
  QGraphicsView::paintEvent(e);
  frames++;
}

void ProjectionGraphicsView::setImage(LaueImage * img) {
  image = img;
}

#include <QPaintEngine>
#include <QImage>

void ProjectionGraphicsView::drawBackground(QPainter *painter, const QRectF &rect) {
  if (image.isNull()) {
    painter->fillRect(rect, Qt::gray);
  } else {
    QTransform t = painter->deviceTransform();
    QRectF sc = sceneRect();
    QPointF p2 = t.map(rect.topRight());
    int w = p2.x();
    int h = p2.y();
    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    QTransform ti = t.inverted();
    for (int x=0; x<w; x++) {
      for (int y=0; y<h; y++) {
        // widget -> scene
        double ix = rect.left()+1.0*x/(w-1)*rect.width();
        double iy = rect.bottom()-1.0*y/(h-1)*rect.height();
        //scene -> image
        ix = (ix-sc.left())/sc.width()*image->width();
        iy = (iy-sc.top())/sc.height()*image->height();

        int iix = int(ix);
        int iiy = int(iy);
        iix = std::min(std::max(0, iix), image->width()-1);
        iiy = std::min(std::max(0, iiy), image->height()-1);

        img.setPixel(x,y,image->getImage().pixel(iix, iiy));
      }
    }

    cout << "Draw BG ";
    cout << p2.x() << " ";
    cout << p2.y() << endl;
    //painter->drawImage(sr, image->getImage());
    painter->save();
    painter->resetTransform();
    painter->drawImage(0,0,img);
    painter->restore();
  }
}
