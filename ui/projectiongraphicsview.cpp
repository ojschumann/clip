#include "projectiongraphicsview.h"

ProjectionGraphicsView::ProjectionGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    QSizePolicy pol = sizePolicy();
    pol.setHeightForWidth(true);
    setSizePolicy(pol);
}


int ProjectionGraphicsView::heightForWidth(int w) const {
    return w;
}
