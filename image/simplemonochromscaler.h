#ifndef SIMPLEMONOCHROMSCALER_H
#define SIMPLEMONOCHROMSCALER_H

#include <image/datascaler.h>

using namespace std;

template <typename T> class SimpleMonochromScaler : public DataScaler
{
public:
  static DataScaler* getScaler(DataProvider*, QObject*);
  virtual void updateContrastMapping();
protected:
  virtual QRgb getRGB(const QPointF &);
private:
  explicit SimpleMonochromScaler(DataProvider* dp, QObject *parent = 0);
  SimpleMonochromScaler(const SimpleMonochromScaler&);
  ~SimpleMonochromScaler();

  class UniqueHelper {
  public:
    UniqueHelper(T _key): key(_key), indexes() {}
    UniqueHelper(const UniqueHelper& o) { key = o.key; indexes=o.indexes; }
    UniqueHelper& operator=(const UniqueHelper& o) { key = o.key; indexes=o.indexes; return *this; }
    T key;
    QList<int> indexes;
    bool operator<(const UniqueHelper& o) const { return key<o.key; }
  };

  void makeValueIndex();
  int datawidth;
  int dataheight;
  QVector<float> unmappedPixelValues;
  QVector<QRgb> mappedPixelValues;
  QVector<int> imagePosToPixelValue;
};




#endif // SIMPLEMONOCHROMSCALER_H
