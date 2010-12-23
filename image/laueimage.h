#ifndef LAUEIMAGE_H
#define LAUEIMAGE_H

#include <QObject>
#include <QImage>
#include <QPointer>
#include <QDomElement>
#include <image/dataprovider.h>
#include <image/datascaler.h>

class ImageToolbox;

class LaueImage : public QObject
{
  Q_OBJECT
public:
  explicit LaueImage(QObject *parent = 0);
  explicit LaueImage(QString, QObject *parent = 0);
  ~LaueImage();

  void saveToXML(QDomElement);
  void loadFromXML(QDomElement);

  QImage getScaledImage(const QSize& , const QRectF&);
  int width() { return provider->width(); }
  int height() { return provider->height(); }
  QSize size() { return QSize(provider->width(), provider->height()); }
  bool isValid() { return (provider!=0) && (scaler!=0); }
  QList<BezierCurve*> getTransferCurves();
  //DataScaler* getScaler() { return scaler; }

  bool hasAbsoluteSize();
  QSizeF absoluteSize(); // in mm

  QString name();
  QList<QWidget*> toolboxPages();


signals:
  void imageContentsChanged();
  void histogramChanged(QVector<int>, QVector<int>, QVector<int>);
public slots:
  void addTransform(const QTransform&);
  void resetAllTransforms();
private:
  void openFile(QString filename);
  DataProvider* provider;
  DataScaler* scaler;
};

#endif // LAUEIMAGE_H
