#ifndef PROPAGATINGGRAPHICSOBJECT_H
#define PROPAGATINGGRAPHICSOBJECT_H

#include <QGraphicsObject>

class PropagatingGraphicsObject : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit PropagatingGraphicsObject(QGraphicsItem *parent = 0);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

signals:

public slots:

};

#endif // PROPAGATINGGRAPHICSOBJECT_H
