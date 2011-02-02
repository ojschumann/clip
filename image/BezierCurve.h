#ifndef __BEZIERCURVE_H__
#define __BEZIERCURVE_H__

#include <QPointF>
#include <QList>
#include <QObject>
#include <QDomElement>


class BezierCurve: public QObject {
  Q_OBJECT
public:
  BezierCurve();
  void saveToXML(QDomElement base, QString name);
  void loadFromXML(QDomElement base, QString name);

  bool setPoints(const QList<QPointF>& p);
  QList<QPointF> getPoints();

  float operator()(float x);
  float operator()(float x, int& hint);

  QList<float> range(float x0, float dx, int N);
  QList<QPointF> pointRange(float x0, float dx, int N);
  QList<float> map(QList<float> X);
  //QList<float> mapSorted(QList<float> X);
  template <class T> T mapSorted(T);
  QList<float> mapSorted(QList<float> X, QList<int> sortIdx);

signals:
  void curveChanged();
private:
  class CurveParams{
  public:
    CurveParams(float D0, float D1, float D2, float D3, float _Xmin, float _Xmax): Xmin(_Xmin), Xmax(_Xmax) {
      D[0]=D0;
      D[1]=D1;
      D[2]=D2;
      D[3]=D3;
    }

    bool operator<(const CurveParams& c) const {return Xmax<c.Xmax; };
    float calc(float x)   {
      float t = D[3];
      for (int n=3; n--; ) {
        t*=x;
        t+=D[n];
      }
      return std::max(0.0f, std::min(1.0f, t));
    }
    float Xmin;
    float Xmax;
            private:
    float D[4];
  };
  CurveParams getCurveParam(float x);

  int getCurveParamIdx(float x);
  QList<CurveParams> params;
  QList<QPointF> points;
};

template <class T> T BezierCurve::mapSorted(T X) {
  T r;
  int p=getCurveParamIdx(X[0]);
  int N=X.size();
  int n=X.size();
  while (n) {
    CurveParams& cp=params[p];
    float x;
    while (n && ((x=X[N-n])<cp.Xmax)) {
      r.append(cp.calc(x));
      n--;
    }
    p++;
  }
  return r;
}


#endif

