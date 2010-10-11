#include "projectiongraphicsview.h"
#include <QMouseEvent>
#include <QTime>

ProjectionGraphicsView::ProjectionGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    viewIgnoresThisMouseEvent=true;
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

void ProjectionGraphicsView::paintEvent(QPaintEvent *e) {
    QTime t=QTime::currentTime();
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
    for (int i=0; i<80000; i++) {
        QSize s = viewport()->size();
        p.drawEllipse(mapFromScene(QPointF(0,0)), 5.0, 5.0);
    }
    p.end();
    delta = t.msecsTo(QTime::currentTime());
    if (delta>0) {
       double ips = 1000.0*20000/delta;
       QPainter p(viewport());

       p.setPen(Qt::black);
       p.drawText(15,45, QString("%1").arg(ips, 9, 'f', 2));
       p.drawText(15,60,QString("%1").arg(delta));
    }

}
