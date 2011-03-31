#include "diagramgv.h"

DiagramGV::DiagramGV(QWidget *parent) :
    QGraphicsView(parent)
{
}

void DiagramGV::resizeEvent(QResizeEvent *event) {
  if (!sceneRect().isEmpty()) {
    fitInView(sceneRect());
  }
}

void DiagramGV::mousePressEvent(QMouseEvent *event) {
  QGraphicsView::mousePressEvent(event);
}

