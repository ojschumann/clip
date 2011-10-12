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

#ifndef SPOTINDICATORGRAPHICSITEM_H
#define SPOTINDICATORGRAPHICSITEM_H

#include <QGraphicsObject>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <QThread>
#include <cmath>

class ThreadRunner;



class SpotIndicatorGraphicsItem: public QGraphicsObject {
  Q_OBJECT
public:
  SpotIndicatorGraphicsItem();
  virtual ~SpotIndicatorGraphicsItem();
private:
  SpotIndicatorGraphicsItem(const SpotIndicatorGraphicsItem&);
  SpotIndicatorGraphicsItem& operator=(const SpotIndicatorGraphicsItem&);
public slots:
  void setColor(QColor);
public:
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  virtual QRectF boundingRect() const;

  void setSpotsize(double s) { spotSize = s; cacheNeedsUpdate=true; }
  void setCachedPainting(bool b=true) { cachedPainting = b; }
  void pointsUpdated();
  QVector<QPointF> coordinates;
protected:
  bool cacheNeedsUpdate;
  bool cachedPainting;
  void updateCache();

  QColor spotColor;

  QPixmap* cache;
  double spotSize;
  QTransform transform;

  class TWorker {
  public:
    typedef QImage CacheType;
    TWorker(SpotIndicatorGraphicsItem* s);
    ~TWorker();
    void operator()(int id);
    void init(int numberOfThreads);
    void done();

    //std::atomic<int> wp;
    QAtomicInt wp;

    SpotIndicatorGraphicsItem* spotIndicator;
    std::vector<CacheType*> threadCaches;
  };

  TWorker tWorker;
  ThreadRunner* threadRunner;
};



#endif // SPOTINDICATORGRAPHICSITEM_H
