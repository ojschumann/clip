#ifndef SPOTINDICATORGRAPHICSITEM_H
#define SPOTINDICATORGRAPHICSITEM_H

#include <QGraphicsObject>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <QThread>

class SpotIndicatorGraphicsItem: public QGraphicsObject {
  Q_OBJECT
public:
  SpotIndicatorGraphicsItem();
  virtual ~SpotIndicatorGraphicsItem();
private:
  SpotIndicatorGraphicsItem(const SpotIndicatorGraphicsItem&);
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

  QSemaphore workerPermission;
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
