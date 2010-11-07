#ifndef LAUEIMAGE_H
#define LAUEIMAGE_H

#include <QObject>
#include <QImage>

class LaueImage : public QObject
{
  Q_OBJECT
public:
  explicit LaueImage(QString, QObject *parent = 0);
  QImage getImage();
  QPixmap getPixmap();
  int width() { return image.width(); };
  int height() { return image.height(); };
  QSize size() { return image.size(); };
  bool isValid() { return valid; }
signals:
  void imageDataChanged();
public slots:
private:
  QImage image;
  bool valid;
};

#endif // LAUEIMAGE_H
