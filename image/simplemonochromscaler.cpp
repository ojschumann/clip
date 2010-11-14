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
  imagePosToPixelValue.resize(provider->pixelCount());
  float minPixelValue = static_cast<float>(indexSet.begin()->key);
  float pixelValueRange = static_cast<float>(indexSet.rbegin()->key) - minPixelValue;

  int n=0;
  foreach (UniqueHelper u, indexSet) {
    unmappedPixelValues[n]=(static_cast<float>(u.key)-minPixelValue)/pixelValueRange;
    foreach (int i, u.indexes)
      imagePosToPixelValue[i]=n;
    n++;
  }

  cout << "setSize: " << indexSet.size() << " " << provider->pixelCount() <<   " t="  << time.msecsTo(QTime::currentTime()) << endl;
  cout << unmappedPixelValues.first() << " " << unmappedPixelValues.last() << endl;
  updateContrastMapping();
}

template <typename T> void SimpleMonochromScaler<T>::updateContrastMapping() {
  mappedPixelValues.resize(unmappedPixelValues.size());
  QVector<float> vMap = transferCurves[0]->mapSorted(unmappedPixelValues);
  for (int n=0; n<mappedPixelValues.size(); n++) {
    unsigned char c = static_cast<unsigned char>(255.0*vMap[n]);
    mappedPixelValues[n]=qRgb(c,c,c);
  }
  redrawCache();
  emit imageContentsChanged();
/*
  QList<float> vMap = curves[0].mapSorted(values);
  QVector<quint32> colors(vMap.size());

  int hints[4];
  for (int i=4; i--; ) hints[i]=0;
  for (int n=vMap.size(); n--; ) {
    float vval=vMap[n];
    int rgbVal=0xFF;
    for (int i=0; i<3; i++) {
      rgbVal<<=8;
      rgbVal|=(int)(255.0*curves[i+1](vval,hints[i+1]));
    }
    colors[n]=rgbVal;
  }
#ifdef __DEBUG__
  cout << "float transfer: Transfer Image" << endl;
#endif
  for (int n=transferedData.size(); n--; ) {
    transferedData[n]=colors[imgData[n]];
  }
}
 */

}



template class SimpleMonochromScaler<float>;

bool SimpleMonochromScaler_Float = DataScalerFactory::registerDataScaler(DataProvider::Float32, &SimpleMonochromScaler<float>::getScaler);
