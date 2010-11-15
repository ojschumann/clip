#ifndef HISTOGRAMITEM_H
#define HISTOGRAMITEM_H

#include <QGraphicsObject>

class HistogramItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit HistogramItem(QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
signals:

public slots:
    void setHistogram(QVector<int>, QVector<int>, QVector<int>);
    void setCompMode(int);
protected:
    QList<QPainterPath> pathes;
    double maxValue;
    int compMode;
};

#endif // HISTOGRAMITEM_H
