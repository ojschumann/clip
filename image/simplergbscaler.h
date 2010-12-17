#ifndef SIMPLERGBSCALER_H
#define SIMPLERGBSCALER_H

#include <QObject>
#include <image/datascaler.h>
#include <image/dataprovider.h>

class SimpleRGBScaler : public DataScaler
{
  Q_OBJECT
public:
  static DataScaler* getScaler(DataProvider*, QObject*);
  ~SimpleRGBScaler();
protected:
  virtual QRgb getRGB(const QPointF &);
private:
  explicit SimpleRGBScaler(DataProvider* dp, QObject *parent = 0);
  SimpleRGBScaler(const SimpleRGBScaler&);
  QRgb const* data;
  int datawidth;
  int dataheight;
};

#endif // SIMPLERGBSCALER_H
