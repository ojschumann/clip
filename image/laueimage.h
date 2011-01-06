#ifndef LAUEIMAGE_H
#define LAUEIMAGE_H

#include <QObject>
#include <QImage>
#include <QPointer>
#include <QDomElement>
#include <QFuture>
#include <QFutureWatcher>

#include "image/dataprovider.h"
#include "image/datascaler.h"

class ImageToolbox;

class LaueImage : public QObject
{
  Q_OBJECT
public:
  explicit LaueImage(QObject *parent = 0);
  virtual ~LaueImage();

  void startOpenFile(QString filename, QDomElement base=QDomElement());

  void saveToXML(QDomElement);
  void loadFromXML(QDomElement);

  void saveCurvesToXML(QDomElement);
  void loadCurvesFromXML(QDomElement, DataScaler* ds=0);

  bool isValid() { return (provider!=0) && (scaler!=0); }

  bool hasAbsoluteSize();
  QSize originalSize();
  QSizeF originalAbsoluteSize(); // in mm
  QSizeF transformedSize();
  QSizeF transformedAbsoluteSize(); // in mm

  QString name();
  QList<QWidget*> toolboxPages();
  QList<BezierCurve*> getTransferCurves();
  QImage getScaledImage(const QSize& , const QRectF&);
signals:
  void imageContentsChanged();
  void histogramChanged(QVector<int>, QVector<int>, QVector<int>);
  void openFinished(LaueImage*);
  void openFailed(LaueImage*);
public slots:
  void addTransform(const QTransform&);
  void resetAllTransforms();
protected slots:
  void doneOpenFile();
protected:
  QPair<DataProvider*, DataScaler*> doOpenFile(QString filename, QDomElement base=QDomElement());
private:
  DataProvider* provider;
  DataScaler* scaler;
  QFutureWatcher< QPair<DataProvider*, DataScaler*> > watcher;

};

#endif // LAUEIMAGE_H
