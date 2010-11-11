#ifndef LAUEIMAGE_H
#define LAUEIMAGE_H

#include <QObject>
#include <QImage>
#include <QPointer>
#include <image/dataprovider.h>
#include <image/datascaler.h>

class ImageToolbox;

class LaueImage : public QObject
{
  Q_OBJECT
public:
  explicit LaueImage(QString, QObject *parent = 0);
  QImage getScaledImage(const QSize& , const QRectF&);
  int width() { return provider->width(); };
  int height() { return provider->height(); };
  QSize size() { return QSize(provider->width(), provider->height()); };
  bool isValid() { return valid; }
signals:
  void imageDataChanged();
public slots:
  void showToolbox();
private:
  QPointer<ImageToolbox> imageToolbox;
  bool valid;
  DataProvider* provider;
  DataScaler* scaler;
};

#endif // LAUEIMAGE_H
