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
protected:
  virtual void redrawCache();
private:
    explicit SimpleRGBScaler(DataProvider* dp, QObject *parent = 0);
    SimpleRGBScaler(const SimpleRGBScaler&);
    ~SimpleRGBScaler();
signals:

public slots:

};

#endif // SIMPLERGBSCALER_H
