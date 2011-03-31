#ifndef DATASCALER_H
#define DATASCALER_H

#include <QObject>
#include <QImage>
#include <QColor>
#include <QDomElement>


class DataProvider;
class BezierCurve;


class DataScaler : public QObject
{
  Q_OBJECT
public:
  explicit DataScaler(DataProvider* dp, QObject *parent = 0);
  virtual ~DataScaler();

  void saveToXML(QDomElement);
  void loadFromXML(QDomElement);

  QImage getImage(const QSize& size, const QPolygonF& from);
  QList<BezierCurve*> getTransferCurves() { return transferCurves; }

signals:
  void imageContentsChanged();
  void histogramChanged(QVector<int>, QVector<int>, QVector<int>);
public slots:
  void addTransform(const QTransform&);
  void resetAllTransforms();
  virtual void updateContrastMapping();
  virtual QList<QWidget*> toolboxPages();
protected:
  QTransform initialTransform();
  virtual void redrawCache();
  virtual QRgb getRGB(const QPointF&)=0;

  DataProvider* provider;
  QImage* cache;
  QPolygonF sourceRect;
  QTransform sqareToRaw;
  QList<BezierCurve*> transferCurves;
};

#endif // DATASCALER_H
