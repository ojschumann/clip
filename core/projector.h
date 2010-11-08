#ifndef __PROJECTOR_H__
#define __PROJECTOR_H__

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
#include <core/fitobject.h>
#include <QWaitCondition>
#include <QSemaphore>
#include <QThread>
#include <QWidget>
#include <QSignalMapper>
#include <tools/vec3D.h>
#include <tools/mat3D.h>
#include <QRunnable>
#include <QMutex>

class Crystal;
class Reflection;
class RulerItem;
class ZoneItem;
class LaueImage;


class Projector: public QObject, public FitObject {
  Q_OBJECT
public:
  Projector(QObject* parent=0);
  ~Projector();

  // Functions for transformations in the different Coordinate systems

  static Vec3D normal2scattered(const Vec3D&);
  static Vec3D normal2scattered(const Vec3D&, bool &b);
  static Vec3D scattered2normal(const Vec3D&);
  static Vec3D scattered2normal(const Vec3D&, bool &b);

  QTransform det2img;
  QTransform img2det;

  virtual QPointF scattered2det(const Vec3D&) const = 0;
  virtual QPointF scattered2det(const Vec3D&, bool& b) const = 0;

  virtual Vec3D det2scattered(const QPointF&) const =0;
  virtual Vec3D det2scattered(const QPointF&, bool& b) const = 0;

  virtual QPointF normal2det(const Vec3D&) const = 0;
  virtual QPointF normal2det(const Vec3D&, bool& b) const = 0;

  virtual Vec3D det2normal(const QPointF&) const = 0;
  virtual Vec3D det2normal(const QPointF&, bool& b) const = 0;

  QGraphicsScene* getScene();
  Crystal* getCrystal();
  virtual QWidget* configWidget()=0;
  LaueImage* getLaueImage();

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

  int spotMarkerNumber() const;
  void addSpotMarker(const QPointF& p);
  void delSpotMarkerNear(const QPointF& p);
  bool delSpotMarkerAt(const QPointF& p);
  QPointF getSpotMarkerDetPos(int n) const;
  QList<Vec3D> getSpotMarkerNormals() const;

  int rulerNumber() const;
  void addRuler(const QPointF& p1, const QPointF& p2);
  bool delRulerAt(const QPointF& p);
  void clearRulers();
  QPair<QPointF, QPointF> getRulerCoordinates(int);
  void highlightRuler(int, bool);
  bool rulerIsHighlighted(int);
  QVariant getRulerData(int);
  void setRulerData(int, QVariant);

  int zoneMarkerNumber() const;
  void addZoneMarker(const QPointF& p1, const QPointF& p2);
  bool delZoneMarkerAt(const QPointF& p);

  void addInfoItem(const QString& text, const QPointF& p);
  bool delInfoItemAt(const QPointF& p);
  void clearInfoItems();
public slots:
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

  void loadImage(QString);
  void closeImage();


  virtual void doImgRotation(int CWRSteps, bool flip);
signals:
  void projectedPointsUpdated();
  void rulerAdded();
  void rulerChanged(int);
  void spotMarkerAdded();
  void spotMarkerChanged();
  void zoneMarkerAdded();
  void wavevectorsUpdated();
  void projectionParamsChanged();
  void projectionRectPosChanged();
  void projectionRectSizeChanged();
  void imgTransformUpdated();
  void imageLoaded(LaueImage*);
protected:
  virtual bool project(const Reflection &r, QPointF &point)=0;
  virtual bool parseXMLElement(QXmlStreamReader&);


  // Stuff like Primary beam marker, Coordinate lines
  QList<QGraphicsItem*> decorationItems;
  // written indexes in the scene
  QList<QGraphicsItem*> textMarkerItems;
  // Markers for indexation and fit
  QList<QGraphicsEllipseItem*> spotMarkerItems;
  // Zone markers
  QList<ZoneItem*> zoneMarkerItems;
  // Info Items. These will be set on Mousepress from Python and be deleted on orientation change or slot!
  QList<QGraphicsItem*> infoItems;
  // Ruler Item
  QList<RulerItem*> rulerItems;
  QSignalMapper rulerMapper;

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
  QGraphicsPixmapItem* imageItemsPlane;
  LaueImage* imageData;

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

    QPixmap* cache;
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
          localCache(0),
          shouldStop(false) {}
      void run();
      SpotMarkerGraphicsItem* spotMarker;
      int threadNr;
      QImage* localCache;
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
private:
  Projector(const Projector&);
};

double getDoubleAttrib(QXmlStreamReader &r, QString name, double def);
int getIntAttrib(QXmlStreamReader &r, QString name, double def);

#endif
