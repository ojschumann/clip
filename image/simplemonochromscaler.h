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

#ifndef SIMPLEMONOCHROMSCALER_H
#define SIMPLEMONOCHROMSCALER_H

#include <image/datascaler.h>

using namespace std;

class AbstractMonoScaler : public DataScaler {
  Q_OBJECT
public:
  explicit AbstractMonoScaler(DataProvider* dp, QObject *_parent= nullptr): DataScaler(dp, _parent) {}
  virtual ~AbstractMonoScaler() {}
protected slots:
  virtual void setHistogramEqualisation(bool)=0;
};

template <typename T> class SimpleMonochromScaler : public AbstractMonoScaler
{
public:
  static DataScaler* getScaler(DataProvider*, QObject*);
  virtual ~SimpleMonochromScaler();
  virtual void updateContrastMapping();
  QList<QWidget*> toolboxPages();
protected:
  virtual QRgb getRGB(const QPointF &);
  virtual void setHistogramEqualisation(bool);
private:
  explicit SimpleMonochromScaler(DataProvider* dp, QObject* _parent = nullptr);
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
