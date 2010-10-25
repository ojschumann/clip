#include <tools/signalingellipse.h>

SignalingEllipseItem::SignalingEllipseItem(QGraphicsItem *parent): QObject(), QGraphicsEllipseItem(parent) {
  skipNextPosChange=false;
};

SignalingEllipseItem::SignalingEllipseItem(const QRectF& rect, QGraphicsItem * parent): QObject(), QGraphicsEllipseItem(rect,parent) {
  skipNextPosChange=false;
};

SignalingEllipseItem::SignalingEllipseItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent): QObject(), QGraphicsEllipseItem(x,y,width,height,parent) {
  skipNextPosChange=false;
};


QVariant SignalingEllipseItem::itemChange(GraphicsItemChange change, const QVariant & value ) {
  if (change==ItemPositionHasChanged) {
    if (not skipNextPosChange) {
      emit positionChanged();
    } else {
      skipNextPosChange=false;
    }
  }
  return QGraphicsEllipseItem::itemChange(change, value);
}

void SignalingEllipseItem::setPosNoSig(const QPointF &p) {
  skipNextPosChange=true;
  setPos(p);
}
