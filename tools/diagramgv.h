#ifndef DIAGRAMGV_H
#define DIAGRAMGV_H

#include <QGraphicsView>

class DiagramGV : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DiagramGV(QWidget *parent = 0);
    void resizeEvent(QResizeEvent *event);
    // Need just to be public
    void mousePressEvent(QMouseEvent *event);
signals:

public slots:

};

#endif // DIAGRAMGV_H

