#include "simplemonochromscaler.h"
#include "image/datascalerfactory.h"


#include <set>
#include <iostream>
#include "image/BezierCurve.h"
#include "image/dataprovider.h"


template <typename T> SimpleMonochromScaler<T>::SimpleMonochromScaler(DataProvider* dp, QObject *parent) :
    DataScaler(dp, parent)
{
  datawidth = dp->width();
  dataheight = dp->height();
  makeValueIndex();

  cout << "init SimpleMonochromeScaler" << endl;
}

template <typename T> SimpleMonochromScaler<T>::SimpleMonochromScaler(const SimpleMonochromScaler &): DataScaler(0)  {}

template <typename T> SimpleMonochromScaler<T>::~SimpleMonochromScaler() {
  cout << "delete SimpleMonochromeScaler" << endl;
}

template <typename T> DataScaler* SimpleMonochromScaler<T>::getScaler(DataProvider *dp, QObject* parent) {
  return new SimpleMonochromScaler(dp, parent);
}

template <typename T> QRgb SimpleMonochromScaler<T>::getRGB(const QPointF &p) {
  int x = static_cast<int>(p.x());
  int y = static_cast<int>(p.y());
  if (x<0 || x>=datawidth || y<0 || y>=dataheight) {
    return 0xFFFF0000;
  } else {
    int idx = imagePosToPixelValue[x+y*datawidth];
    return mappedPixelValues[idx];
  }
}

#include <QTime>


template <typename T> void SimpleMonochromScaler<T>::makeValueIndex() {
  T const* mydata = static_cast<T const*>(provider->getData());
  set< UniqueHelper > indexSet;
  pair<typename set< UniqueHelper >::iterator, bool> res;
  QTime time=QTime::currentTime();
  for (int i=0; i<provider->pixelCount(); i++) {
    res = indexSet.insert(UniqueHelper(*mydata));
    const_cast<QList<int>* >(&res.first->indexes)->append(i);
    mydata++;
  }

  unmappedPixelValues.resize(indexSet.size());
  mappedPixelValues.resize(indexSet.size());
  valueCount.resize(indexSet.size());
  imagePosToPixelValue.resize(provider->pixelCount());

  float minPixelValue = static_cast<float>(indexSet.begin()->key);
  float pixelValueRange = static_cast<float>(indexSet.rbegin()->key) - minPixelValue;

  int n=0;
  foreach (UniqueHelper u, indexSet) {
    unmappedPixelValues[n]=(static_cast<float>(u.key)-minPixelValue)/pixelValueRange;
    valueCount[n] = u.indexes.size();
    foreach (int i, u.indexes)
      imagePosToPixelValue[i]=n;
    n++;
  }

  cout << "setSize: " << indexSet.size() << " " << provider->pixelCount() <<   " t="  << time.msecsTo(QTime::currentTime()) << endl;
  cout << unmappedPixelValues.first() << " " << unmappedPixelValues.last() << endl;
  updateContrastMapping();
}

template <typename T> void SimpleMonochromScaler<T>::updateContrastMapping() {
  QList<QVector<int> > channels;
  channels << QVector<int>(256) << QVector<int>(256) << QVector<int>(256);

  QVector<float> vMap = transferCurves[0]->mapSorted(unmappedPixelValues);
  int Hints[3] = {0, 0, 0};
  float scale = 256.0*(1.0-1.0/mappedPixelValues.size());
  for (int n=0; n<vMap.size(); n++) {
    float v = vMap[n];
    QRgb color=0xFF;
    for (int i=0; i<3; i++) {
      int c = static_cast<int>(scale*(*transferCurves[i+1])(v, Hints[i]));
      color <<= 8;
      color |= c;
      channels[i][c]+=valueCount[n];
    }
    mappedPixelValues[n]=color;
  }
  redrawCache();
  emit imageContentsChanged();
  emit histogramChanged(channels[0], channels[1], channels[2]);
}



template class SimpleMonochromScaler<float>;

bool SimpleMonochromScaler_Float = DataScalerFactory::registerDataScaler(DataProvider::Float32, &SimpleMonochromScaler<float>::getScaler);
