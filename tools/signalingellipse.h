#ifndef __SIGNALING_ELLIPSE_H__
#define __SIGNALING_ELLIPSE_H__

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsEllipseItem>
#include <QtCore/QVariant>

class SignalingEllipseItem: public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
    public:
        SignalingEllipseItem(QGraphicsItem * parent=0);
        SignalingEllipseItem(const QRectF& rect, QGraphicsItem * parent=0);
        SignalingEllipseItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent=0);
        void setPosNoSig(const QPointF& p);
    protected:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value );
    signals:
        void positionChanged();
    private:
        bool skipNextPosChange;
};

#endif
