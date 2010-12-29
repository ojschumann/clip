#ifndef SIMPLEMONOCHROMSCALER_H
#define SIMPLEMONOCHROMSCALER_H

#include <image/datascaler.h>

using namespace std;

class AbstractMonoScaler : public DataScaler {
  Q_OBJECT
public:
  explicit AbstractMonoScaler(DataProvider* dp, QObject *parent = 0): DataScaler(dp, parent) {}
  ~AbstractMonoScaler() {}
protected slots:
  virtual void setHistogramEqualisation(bool)=0;
};

template <typename T> class SimpleMonochromScaler : public AbstractMonoScaler
{
public:
  static DataScaler* getScaler(DataProvider*, QObject*);
  ~SimpleMonochromScaler();
  virtual void updateContrastMapping();
  QList<QWidget*> toolboxPages();
protected:
  virtual QRgb getRGB(const QPointF &);
  virtual void setHistogramEqualisation(bool);
private:
  explicit SimpleMonochromScaler(DataProvider* dp, QObject *parent = 0);
  SimpleMonochromScaler(const SimpleMonochromScaler&);
  void makeValueIndex();

  class UniqueHelper {
  public:
    UniqueHelper(T _key): key(_key), indexes() {}
    UniqueHelper(const UniqueHelper& o) { key = o.key; indexes=o.indexes; }
    UniqueHelper& operator=(const UniqueHelper& o) { key = o.key; indexes=o.indexes; return *this; }
    void addIndex(int i) const { const_cast<QList<int>& >(indexes).append(i); }
    T key;
    QList<int> indexes;
    inline bool operator<(const UniqueHelper& o) const { return key<o.key; }
    inline bool operator==(const UniqueHelper& o) const { return key==o.key; }
  };
  int datawidth;
  int dataheight;

  struct hash {
    std::size_t operator()(const UniqueHelper& v) const { return (std::size_t)v.key; }
  };

  bool histogramEqualisation;

  // Vector of the original, ordered and distinct pixel values scaled to 0..1
  QVector<float> unmappedPixelValues;
  // cummulative Histogram scaled to 0..1
  QVector<float> cummulativeHistogram;
  // Number of values per Pixel count (basically a histogram)
  QVector<int> valueCount;
  // Mapped pixel values
  QVector<QRgb> mappedPixelValues;
  // mappes index of a pixel to its value in unmappedPixelValues and mappedPixelValues
  QVector<int> imagePosToPixelValue;
};




#endif // SIMPLEMONOCHROMSCALER_H
