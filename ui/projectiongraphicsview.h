#ifndef PROJECTIONGRAPHICSVIEW_H
#define PROJECTIONGRAPHICSVIEW_H

#include <QGraphicsView>

class ProjectionGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ProjectionGraphicsView(QWidget *parent = 0);
    virtual int heightForWidth(int) const;
signals:

public slots:

};

#endif // PROJECTIONGRAPHICSVIEW_H
