#ifndef PROPAGATINGGRAPHICSOBJECT_H
#define PROPAGATINGGRAPHICSOBJECT_H

#include <QGraphicsObject>
#include <QVariant>

class PropagatingGraphicsObject : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit PropagatingGraphicsObject(QGraphicsItem *parent = 0);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void setImgTransform(const QTransform&);
protected:
    bool propagatePositionChange;
    bool propagateTransformChange;
    QTransform propagatedTransform;

signals:

public slots:

};

#endif // PROPAGATINGGRAPHICSOBJECT_H
