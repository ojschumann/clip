#ifndef DATASCALER_H
#define DATASCALER_H

#include <QObject>
#include <QImage>
#include <QColor>


class DataProvider;
class BezierCurve;


class DataScaler : public QObject
{
  Q_OBJECT
public:
  explicit DataScaler(DataProvider* dp, QObject *parent = 0);
  ~DataScaler();
  QImage getImage(const QSize& size, const QRectF& from);
  QList<BezierCurve*> getTransferCurves() { return transferCurves; }
signals:
  void imageContentsChanged();
public slots:
  void addTransform(const QTransform&);
  void resetAllTransforms();
  virtual void updateContrastMapping();
protected:
  virtual void redrawCache();
  virtual QRgb getRGB(const QPointF&)=0;
  DataProvider* provider;
  QImage* cache;
  QRectF sourceRect;
  QTransform sqareToRaw;
  QList<BezierCurve*> transferCurves;
};

#endif // DATASCALER_H
