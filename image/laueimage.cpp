#include "laueimage.h"
#include <QImage>
#include <QPixmap>
#include <ui/imagetoolbox.h>
#include <ui/clip.h>
#include <QApplication>

LaueImage::LaueImage(QString s, QObject *parent) :
    QObject(parent), image()
{
  valid = image.load(s);
  image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

QImage LaueImage::getImage() {
  return image;
}

void LaueImage::showToolbox() {
  if (imageToolbox.isNull()) {
    imageToolbox = new ImageToolbox(this);
    connect(this, SIGNAL(destroyed()), imageToolbox.data(), SLOT(deleteLater()));
    Clip::getInstance().addMdiWindow(imageToolbox);
  } else {
    Clip::getInstance().setActiveSubWindow(imageToolbox);
  }
}


QImage LaueImage::getScaledImage(const QSize& requestedSize, const QRectF& r) {
  int w = requestedSize.width();
  int h = requestedSize.height();

  QImage img(requestedSize, image.format());

  double x0 = r.left()*image.width();
  double dx = r.width()*image.width()/img.width();

  double y0 = r.top()*image.height();
  double dy = r.height()*image.height()/img.height();

  QRgb* data = (QRgb*)img.bits();

  double fy = y0;
  int liy = -1;
  for (int y=0; y<h; y++) {
    int iy = std::min(std::max(0, int(fy)), image.height()-1);
    fy += dy;
    if (liy==iy) {
      memcpy(data, data-w, img.bytesPerLine());
      data += w;
    } else {
      QRgb* source = (QRgb*)image.scanLine(iy);
      double fx = x0;
      for (int x=0; x<w; x++) {
        int ix = std::min(std::max(0, int(fx)), image.width()-1);
        fx += dx;
        *data = *(source+ix);
        data++;
      }
    }
    liy = iy;
  }
  return img;
}

