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

#ifndef LAUEIMAGE_H
#define LAUEIMAGE_H

#include <QObject>
#include <QImage>
#include <QPointer>
#include <QDomElement>
#include <QFuture>
#include <QFutureWatcher>

#include "image/imagedatastore.h"

class DataProvider;
class DataScaler;
class BezierCurve;

class LaueImage : public QObject
{
  Q_OBJECT
public:


  explicit LaueImage(QObject* _parent = nullptr);
  virtual ~LaueImage();

  void startOpenFile(QString filename, QDomElement base=QDomElement());

  void saveToXML(QDomElement);
  void loadFromXML(QDomElement);

  void saveCurvesToXML(QDomElement);
  void loadCurvesFromXML(QDomElement, DataScaler* ds=nullptr);

  bool isValid() { return (provider!=nullptr) && (scaler!=nullptr); }

  QString name();
  QList<QString> infoKeys();
  bool hasInfo(const QString& key);
  QVariant getInfo(const QString& key);

  QList<QWidget*> toolboxPages();
  QList<BezierCurve*> getTransferCurves();
  QImage getScaledImage(const QSize& , const QPolygonF&);

  ImageDataStore* data() { return &dataStore; }
signals:
  void imageContentsChanged();
  void histogramChanged(QVector<int>, QVector<int>, QVector<int>);
  void openFinished(LaueImage*);
  void openFailed(LaueImage*);

public slots:
  void addTransform(const QTransform&);
  void resetAllTransforms();
protected slots:
  void doneOpenFile();
protected:
  QPair<DataProvider*, DataScaler*> doOpenFile(QString filename, QDomElement base=QDomElement());
private:
  DataProvider* provider;
  DataScaler* scaler;
  QFutureWatcher< QPair<DataProvider*, DataScaler*> > watcher;

  ImageDataStore dataStore;

};

#endif // LAUEIMAGE_H
