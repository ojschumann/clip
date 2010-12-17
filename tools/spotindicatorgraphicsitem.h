#ifndef SPOTINDICATORGRAPHICSITEM_H
#define SPOTINDICATORGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <QThread>

class SpotIndicatorGraphicsItem: public QGraphicsItem {
public:
  SpotIndicatorGraphicsItem();
  ~SpotIndicatorGraphicsItem();
private:
  SpotIndicatorGraphicsItem(const SpotIndicatorGraphicsItem&);
public:
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  virtual QRectF boundingRect() const;

  void setSpotsize(double s) { spotSize = s; cacheNeedsUpdate=true; }
  void pointsUpdated();
  QVector<QPointF> coordinates;
  int paintUntil;
protected:
  bool cacheNeedsUpdate;
  void updateCache();

  QPixmap* cache;
  double spotSize;
  QTransform transform;

  QMutex mutex;
  QWaitCondition workerStart;
  QSemaphore workerSync;
  QAtomicInt workN;

  class Worker: public QThread {
  public:
    Worker(SpotIndicatorGraphicsItem* s, int t):
        spotIndicator(s),
        threadNr(t),
        localCache(0),
        shouldStop(false) {}
    void run();
    SpotIndicatorGraphicsItem* spotIndicator;
    int threadNr;
    QImage* localCache;
    bool shouldStop;
  private:
    Worker(const Worker&) {};
  };
  QList<Worker*> workers;
};

#endif // SPOTINDICATORGRAPHICSITEM_H
