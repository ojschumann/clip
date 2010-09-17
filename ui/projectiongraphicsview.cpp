#include "projectiongraphicsview.h"
#include <QMouseEvent>

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

