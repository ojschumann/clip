#include "laueimage.h"
#include <QImage>
#include <QPixmap>

LaueImage::LaueImage(QString s, QObject *parent) :
    QObject(parent)
{
  valid = image.load(s);
}

QImage LaueImage::getImage() {
  return image;
}

QPixmap LaueImage::getPixmap() {
  return QPixmap::fromImage(image);
}
