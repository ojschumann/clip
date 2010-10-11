#ifndef PROJECTIONGRAPHICSVIEW_H
#define PROJECTIONGRAPHICSVIEW_H

#include <QGraphicsView>

class ProjectionGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ProjectionGraphicsView(QWidget *parent = 0);

    void dragEnterEvent(QDragEnterEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent*);
    bool viewIgnoresThisMouseEvent;

};

#endif // PROJECTIONGRAPHICSVIEW_H
