#ifndef XMLTOOLS_H
#define XMLTOOLS_H

#include <QDomElement>
#include <QString>
#include <QRectF>
#include <QRect>

template <class R> void RectToTag(QDomElement base, QString name, const R& rect);


QRectF TagToRect(QDomElement element, const QRectF& defaultValue, bool* _ok=0);
QRect TagToRect(QDomElement element, const QRect& defaultValue, bool* _ok=0);

void PointToTag(QDomElement base, QString name, const QPointF& p);
QPointF TagToPoint(QDomElement element, const QPointF& defaultValue, bool* _ok=0);

QDomElement ensureElement(QDomElement base, QString name);

int    readInt   (QDomElement element, QString name, bool& ok, int    defaultValue=0);
double readDouble(QDomElement element, QString name, bool& ok, double defaultValue=0);

#endif // XMLTOOLS_H
