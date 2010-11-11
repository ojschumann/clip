#include "laueimage.h"
#include <QImage>
#include <QPixmap>
#include <ui/imagetoolbox.h>
#include <ui/clip.h>
#include <QApplication>
#include <image/dataproviderfactory.h>
#include <image/datascalerfactory.h>


LaueImage::LaueImage(QString s, QObject *parent) :
    QObject(parent), provider(0), scaler(0)
{
  provider = DataProviderFactory::getInstance().loadImage(s);
  if (provider) {
    scaler = DataScalerFactory::getInstance().getScaler(provider);
    provider->setParent(this);
    if (scaler) scaler->setParent(this);
  }
  valid = (provider!=0) && (scaler!=0);
}

void LaueImage::showToolbox() {
  if (imageToolbox.isNull()) {
    imageToolbox = new ImageToolbox(this);
    connect(this, SIGNAL(destroyed()), imageToolbox.data(), SLOT(deleteLater()));
    Clip::getInstance()->addMdiWindow(imageToolbox);
  } else {
    Clip::getInstance()->setActiveSubWindow(imageToolbox);
  }
}

QImage LaueImage::getScaledImage(const QSize& requestedSize, const QRectF& r) {
  return scaler->getImage(requestedSize, r);
}

