#include "projectiongraphicsview.h"
#include <QMouseEvent>
#include <QTime>
#include <QGraphicsItem>

ProjectionGraphicsView::ProjectionGraphicsView(QWidget *parent) :
    QGraphicsView(parent) {
  viewIgnoresThisMouseEvent=true;
  frames=0;
}

void ProjectionGraphicsView::dragEnterEvent(QDragEnterEvent *e) {
  if (!not e->mimeData()->hasFormat("application/CrystalPointer"))
    QGraphicsView::dragEnterEvent(e);
  e->ignore();
}

void ProjectionGraphicsView::mousePressEvent(QMouseEvent *e) {
  if (e->buttons()==Qt::LeftButton) {
    QGraphicsView::mousePressEvent(e);
    viewIgnoresThisMouseEvent = !e->isAccepted();
  } else {
    viewIgnoresThisMouseEvent=true;
    e->ignore();
  }
}

void ProjectionGraphicsView::mouseMoveEvent(QMouseEvent *e) {
  if (viewIgnoresThisMouseEvent) {
    e->ignore();
  } else {
    QGraphicsView::mouseMoveEvent(e);
  }
}

void ProjectionGraphicsView::mouseReleaseEvent(QMouseEvent *e) {
  if (viewIgnoresThisMouseEvent) {
    e->ignore();
  } else {
    QGraphicsView::mouseMoveEvent(e);
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
  /*QTime t=QTime::currentTime();
    QGraphicsView::paintEvent(e);
    int delta = t.msecsTo(QTime::currentTime());
    if (delta>0) {
       double ips = 1000.0*scene()->items().size()/delta;
       QPainter p(viewport());

       p.setPen(Qt::black);
       p.drawText(15, 15, QString("%1").arg(ips, 9, 'f', 2));
       p.drawText(15,30,QString("%1").arg(delta));
    }
    t=QTime::currentTime();
    QPainter p(viewport());
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QBrush(Qt::black), 0));
    QPainterPath path;
    path.addEllipse(-4.0, -4.0, 8.0, 8.0);
    QPolygonF poly = path.toFillPolygon();
    QPointF l;
    for (int i=0; i<scene()->items().size(); i++) {

        QGraphicsItem* it = scene()->items().at(i);
        QPointF pos = mapFromScene(it->pos() );
        //p.drawEllipse(pos, 2.5, 2.5);
        poly.translate(pos-l);
        l=pos;
        p.drawPolygon(poly);
    }
    p.end();
    delta = t.msecsTo(QTime::currentTime());
    if (delta>0) {
       double ips = 1000.0*scene()->items().size()/delta;
       QPainter p(viewport());

       p.setPen(Qt::black);
       p.drawText(15,45, QString("%1").arg(ips, 9, 'f', 2));
       p.drawText(15,60,QString("%1").arg(delta));
    }
*/
}
