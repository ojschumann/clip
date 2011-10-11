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

#ifndef DATASCALER_H
#define DATASCALER_H

#include <QObject>
#include <QImage>
#include <QColor>
#include <QDomElement>

#include "config.h"

class DataProvider;
class BezierCurve;


class DataScaler : public QObject
{
  Q_OBJECT
public:
  explicit DataScaler(DataProvider* dp, QObject* _parent= nullptr);
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
