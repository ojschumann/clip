#ifndef DATASCALER_H
#define DATASCALER_H

#include <QObject>
#include <QImage>

class DataProvider;

class DataScaler : public QObject
{
  Q_OBJECT
public:
  explicit DataScaler(DataProvider* dp, QObject *parent = 0);
  ~DataScaler();
  QImage getImage(const QSize& size, const QRectF& from);
signals:
  void imageContentsChanged();
public slots:
  void addTransform(const QTransform&);
protected:
  virtual void redrawCache()=0;
  DataProvider* provider;
  QImage* cache;
  QRectF sourceRect;
  QTransform sqareToRaw;
};

#endif // DATASCALER_H
