/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

#include "simplemonochromscaler.h"

#include <set>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
 
#include <cmath>

#include "image/beziercurve.h"
#include "image/datascalerfactory.h"


template <typename T> SimpleMonochromScaler<T>::SimpleMonochromScaler(DataProvider* dp, QObject* _parent) :
    AbstractMonoScaler(dp, _parent)
{
  histogramEqualisation = false;
  datawidth = dp->size().width();
  dataheight = dp->size().height();
  makeValueIndex();
}

template <typename T> SimpleMonochromScaler<T>::SimpleMonochromScaler(const SimpleMonochromScaler &): AbstractMonoScaler(0)  {}

template <typename T> SimpleMonochromScaler<T>::~SimpleMonochromScaler() {
}

template <typename T> DataScaler* SimpleMonochromScaler<T>::getScaler(DataProvider *dp, QObject* _parent) {
  return new SimpleMonochromScaler(dp, _parent);
}

template <typename T> QRgb SimpleMonochromScaler<T>::getRGB(const QPointF &p) {
  int x = static_cast<int>(std::floor(p.x()));
  int y = static_cast<int>(std::floor(p.y()));
  if (x<0 || x>=datawidth || y<0 || y>=dataheight) {
    return 0xFF000000;
  } else {
    int idx = imagePosToPixelValue[x+y*datawidth];
    return mappedPixelValues[idx];
  }
}





template <typename T> void SimpleMonochromScaler<T>::makeValueIndex() {

  T const* mydata = static_cast<T const*>(provider->getData());

  // Generates a hashed Tree with the distinct pixel values in the image.
  // store the index of each pixel for every pixel value
  set<UniqueHelper> indexSet;
  std::tr1::unordered_set<UniqueHelper, hash> uniqueSet;
  // loop over all pixel
  for (int i=0; i<provider->pixelCount(); i++) {
    // Try to insert pixel, res containes an iterator to the element and
    // info if this is a new pixel value
    uniqueSet.insert(UniqueHelper(*mydata)).first->addIndex(i);
    mydata++;
  }
  foreach (UniqueHelper u, uniqueSet) indexSet.insert(u);


  // indexSet.size() is now the number of distinct pixel values
  unmappedPixelValues.resize(indexSet.size());
  cummulativeHistogram.resize(indexSet.size());
  mappedPixelValues.resize(indexSet.size());
  valueCount.resize(indexSet.size());
  imagePosToPixelValue.resize(provider->pixelCount());

  float minPixelValue = static_cast<float>(indexSet.begin()->key);
  float pixelValueRange = static_cast<float>(indexSet.rbegin()->key) - minPixelValue;

  int n=0;
  int histogramSum=0;
  float cumHistScale = 1.0/(provider->pixelCount() - indexSet.rbegin()->indexes.size());
  foreach (UniqueHelper u, indexSet) {
    unmappedPixelValues[n]=(static_cast<float>(u.key)-minPixelValue)/pixelValueRange;
    cummulativeHistogram[n] = cumHistScale*histogramSum;
    valueCount[n] = u.indexes.size();
    histogramSum += valueCount[n];
    foreach (int i, u.indexes)
      imagePosToPixelValue[i]=n;
    n++;
  }

  updateContrastMapping();
}

template <typename T> void SimpleMonochromScaler<T>::updateContrastMapping() {
  QList<QVector<int> > channels;
  channels << QVector<int>(256) << QVector<int>(256) << QVector<int>(256);

  QVector<float> vMap = transferCurves[0]->mapSorted((histogramEqualisation) ? cummulativeHistogram : unmappedPixelValues);
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

template <typename T> QList<QWidget*> SimpleMonochromScaler<T>::toolboxPages() {
  QList<QWidget*> pages;

  /*QCheckBox* b = new QCheckBox("Histogram Equilisation");
  b->setChecked(histogramEqualisation);
  b->setObjectName("Scaler");
  connect(b, SIGNAL(toggled(bool)), this, SLOT(setHistogramEqualisation(bool)));
  pages << b;
  */
  return pages;
}

template <typename T> void SimpleMonochromScaler<T>::setHistogramEqualisation(bool b) {
  if (b!=histogramEqualisation) {
    histogramEqualisation = b;
    updateContrastMapping();
  }
}

template class SimpleMonochromScaler<float>;
template class SimpleMonochromScaler<unsigned int>;

bool SimpleMonochromScaler_Float = DataScalerFactory::registerDataScaler(DataProvider::Float32, &SimpleMonochromScaler<float>::getScaler);
bool SimpleMonochromScaler_Int = DataScalerFactory::registerDataScaler(DataProvider::UInt32, &SimpleMonochromScaler<unsigned int>::getScaler);
