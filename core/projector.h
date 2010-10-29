#ifndef __PROJECTOR_H__
#define __PROJECTOR_H__

#include <core/reflection.h>
#include <QObject>
#include <QVector>
#include <QPointF>
#include <QPointer>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <core/crystal.h>
#include <core/fitobject.h>
#include <QWaitCondition>
#include <QSemaphore>
#include <QThread>
#include <QWidget>
#include <tools/ruleritem.h>

class Projector: public QObject, public FitObject {
  Q_OBJECT
public:
  Projector(QObject* parent=0);
  Projector(const Projector&);
  ~Projector();

  // Functions for transformations in the different Coordinate systems

  static Vec3D normal2scattered(const Vec3D&, bool* b=NULL);
  static Vec3D scattered2normal(const Vec3D&, bool* b=NULL);

  QTransform det2img;
  QTransform img2det;

  virtual QPointF scattered2det(const Vec3D&, bool* b=NULL) const =0;
  virtual Vec3D det2scattered(const QPointF&, bool* b=NULL) const =0;
  virtual QPointF normal2det(const Vec3D&, bool* b=NULL) const =0;
  virtual Vec3D det2normal(const QPointF&, bool* b=NULL) const =0;

  QGraphicsScene* getScene();
  Crystal* getCrystal();
  virtual QString projectorName()=0;
  virtual QString displayName()=0;

  double Qmin() const;
  double Qmax() const;

  virtual double TTmin() const;
  virtual double TTmax() const;

  int getMaxHklSqSum() const;
  double getTextSize() const;
  double getSpotSize() const;
  bool spotsEnabled() const;


  virtual void projector2xml(QXmlStreamWriter&);
  virtual void loadFromXML(QXmlStreamReader&);

    public slots:
  virtual QWidget* configWidget()=0;
  void connectToCrystal(Crystal *);

  // Set Wavevectors. Note that the Value is 1/lambda, not 2*pi/lambda
  void setWavevectors(double Qmin, double Qmax);
  void reflectionsUpdated();

  void addRotation(const Vec3D &axis, double angle);
  void addRotation(const Mat3D& M);
  void setRotation(const Mat3D& M);

  virtual void decorateScene()=0;
  void setMaxHklSqSum(int m);
  void setTextSize(double d);
  void setSpotSize(double d);
  void enableSpots(bool b=true);
  // For speedup of fitting...
  void enableProjection(bool b=true);

  int markerNumber() const;
  void addMarker(const QPointF& p);
  void delMarkerNear(const QPointF& p);
  QPointF getMarkerDetPos(int n) const;
  QList<Vec3D> getMarkerNormals() const;

  int rulerNumber() const;
  void addRuler(const QPointF& p1, const QPointF& p2);
  void updateMostRecentRuler(const QPointF& p);
  QPair<QPointF, QPointF> getRulerCoordinates(int);
  double getRulerSize(int);
  void setRulerSize(int, double);

  virtual void doImgRotation(int CWRSteps, bool flip);

  void addInfoItem(const QString& text, const QPointF& p);
  void clearInfoItems();

signals:
  void projectedPointsUpdated();
  void rulersAdded();
  void wavevectorsUpdated();
  void projectionParamsChanged();
  void projectionRectPosChanged();
  void projectionRectSizeChanged();
  void imgTransformUpdated();
protected:
  virtual bool project(const Reflection &r, QPointF &point)=0;
  virtual bool parseXMLElement(QXmlStreamReader&);


  // Stuff like Primary beam marker, Coordinate lines
  QList<QGraphicsItem*> decorationItems;
  // written indexes in the scene
  QList<QGraphicsItem*> textMarkerItems;
  // Markers for indexation and fit
  QList<QGraphicsEllipseItem*> markerItems;
  // Info Items. These will be set on Mousepress from Python and be deleted on orientation change or slot!
  QList<QGraphicsItem*> infoItems;
  // Ruler Item
  QList<RulerItem*> rulerItems;

  // Pointer to the connected crystal
  QPointer<Crystal> crystal;

  double QminVal;
  double QmaxVal;
  int maxHklSqSum;
  double textSize;
  double spotSize;
  bool showSpots;
  bool projectionEnabled;

  QGraphicsScene scene;
  QGraphicsPixmapItem* imgGroup;
  QGraphicsPixmapItem* img;

  class ProjectionMapper: public QRunnable {
  public:
    ProjectionMapper(Projector* p, QVector<Reflection> r);
    ~ProjectionMapper();
    void run();
  private:
    Projector* projector;
    QVector<Reflection> reflections;
    QAtomicInt nextReflection;
    QAtomicInt nextUnusedPoint;
    QMutex mutex;
  };

  class SpotMarkerGraphicsItem: public QGraphicsItem {
  public:
    SpotMarkerGraphicsItem();
    ~SpotMarkerGraphicsItem();
  private:
    SpotMarkerGraphicsItem(const SpotMarkerGraphicsItem&);
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

    QPixmap cache;
    double spotSize;
    QTransform transform;

    QMutex mutex;
    QWaitCondition workerStart;
    QSemaphore workerSync;
    QAtomicInt workN;

    class Worker: public QThread {
    public:
      Worker(SpotMarkerGraphicsItem* s, int t):
          spotMarker(s),
          threadNr(t),
          localCache(s->cache.size(), QImage::Format_ARGB32_Premultiplied),
          shouldStop(false) {}
      void run();
      SpotMarkerGraphicsItem* spotMarker;
      int threadNr;
      QImage localCache;
      bool shouldStop;
    private:
      Worker(const Worker&) {};
    };
    QList<Worker*> workers;
  };

  SpotMarkerGraphicsItem* spotMarkers;
    protected slots:
  virtual void updateImgTransformations();
protected:
  void internalSetWavevectors(double, double);
};

double getDoubleAttrib(QXmlStreamReader &r, QString name, double def);
int getIntAttrib(QXmlStreamReader &r, QString name, double def);

#endif
