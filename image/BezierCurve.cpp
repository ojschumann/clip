#include <image/BezierCurve.h>
#include <iostream>
#include <cmath>

using namespace std;

BezierCurve::BezierCurve(): params() {
  QList<QPointF> p;
  p.append(QPointF(0,0));
  p.append(QPointF(1,1));
  setPoints(p);
};

bool BezierCurve::setPoints(QList<QPointF>& p) {
  points=p;
  // this ist stolen from GIMP. Hale the GPL!!!
  // File: GIMP/app/base/curves.c
  params.clear();
  if (p.size()<2) {
    return false;
  } else if (p.size()==2) {
    float dx=p.at(1).x()-p.at(0).x();
    if (dx==0.0)
      return false;
    float slope=(p.at(1).y()-p.at(0).y())/dx;
    params.append(CurveParams(p.at(0).y()-slope*p.at(0).x(), slope, 0.0, 0.0, -INFINITY, INFINITY));
  } else {
    for (int i=p.size()-1; i--; ) {
      float p0=p.at(i).y();
      float p3=p.at(i+1).y();
      float p1;
      float p2;
      float dx=p.at(i+1).x()-p.at(i).x();
      if (dx==0.0)
        continue;

      if (i==0) {
        //only the right neighbor is available. Make the tangent at the
        //right endpoint parallel to the line between the left endpoint
        //and the right neighbor. Then point the tangent at the left towards
        //the control handle of the right tangent, to ensure that the curve
        //does not have an inflection point.
        float slope = (p.at(i+2).y() - p.at(i).y()) / (p.at(i+2).x() - p.at(i).x());
        p2 = p.at(i+1).y() - slope * dx / 3.0;
        p1 = p.at(i).y() + (p2 - p.at(i).y()) / 2.0;
      } else if (i==p.size()-2) {
        float slope = (p.at(i+1).y() - p.at(i-1).y()) / (p.at(i+1).x() - p.at(i-1).x());
        p1 = p.at(i).y() + slope * dx / 3.0;
        p2 = p.at(i+1).y() + (p1 - p.at(i+1).y()) / 2.0;
      } else {
        // Both neighbors are available. Make the tangents at the endpoints
        // parallel to the line between the opposite endpoint and the adjacent
        //neighbor.
        float slope = (p.at(i+1).y() - p.at(i-1).y()) / (p.at(i+1).x() - p.at(i-1).x());
        p1 = p.at(i).y() + slope * dx / 3.0;
        slope = (p.at(i+2).y() - p.at(i).y()) / (p.at(i+2).x() - p.at(i).x());
        p2 = p.at(i+1).y() - slope * dx / 3.0;
      }
      float C0=p0;
      float C1=(        3*p1-3*p0)/dx;
      float C2=(   3*p2-6*p1+3*p0)/dx/dx;
      float C3=(p3-3*p2+3*p1-  p0)/dx/dx/dx;

      float x=p.at(i).x();

      float D0 = ((-C3*x +C2)*x-C1)*x+C0;
      float D1 = (3.0*C3*x -2.0*C2)*x+C1;
      float D2=C2 - 3.0*C3*x;
      float D3=C3;

      params.prepend(CurveParams(D0,D1,D2,D3,p.at(i).x(),p.at(i+1).x()));
    }
    params.first().Xmin=-INFINITY;
    params.last().Xmax=INFINITY;
    return not params.empty();
  }
  return true;
}

QList<QPointF> BezierCurve::getPoints() {
  return points;
}

float BezierCurve::operator()(float x) {
  if (params.empty())
    return 0.0;

  int p=getCurveParamIdx(x);
  return params[p].calc(x);
}

float BezierCurve::operator()(float x, int& hint) {
  while (params[hint].Xmax<x) hint++;
  while (params[hint].Xmin>x) hint--;
  return params[hint].calc(x);
}


QList<float> BezierCurve::range(float x0, float dx, int N) {
  QList<float> r;
  float x=x0;
  int p=getCurveParamIdx(x);
  int n=N;
  while (n) {
    CurveParams& cp=params[p];
    while (n && (x<cp.Xmax)) {
      r.append(cp.calc(x));
      x+=dx;
      n--;
    }
    p++;
  }
  return r;
}

QList<QPointF> BezierCurve::pointRange(float x0, float dx, int N) {
  QList<QPointF> r;
  float x=x0;
  int p=getCurveParamIdx(x);
  int n=N;
  while (n) {
    CurveParams& cp=params[p];
    while (n && (x<cp.Xmax)) {
      r.append(QPointF(x,cp.calc(x)));
      x+=dx;
      n--;
    }
    p++;
  }
  return r;
}


QList<float> BezierCurve::map(QList<float> X) {
  QList<float> r;
  for (int n=X.size(); n--; ) {
    float x=X[n];
    int p=getCurveParamIdx(x);
    r.prepend(params[p].calc(x));
  }
  return r;
}

QList<float> BezierCurve::mapSorted(QList<float> X) {
  QList<float> r;
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

QList<float> BezierCurve::mapSorted(QList<float> X, QList<int> Idx) {
  QList<float> r(X);
  int p=getCurveParamIdx(X[Idx[0]]);
  int n=X.size();
  while (n) {
    CurveParams& cp=params[p];
    float x;
    int idx=Idx[n-1];
    while (n && ((x=X[idx])<cp.Xmax)) {
      r[idx]=cp.calc(x);
      n--;
    }
    p++;
  }
  return r;

}

int BezierCurve::getCurveParamIdx(float x) {
  CurveParams cp(0,0,0,0,0,x);
  QList<CurveParams>::const_iterator iter=qLowerBound(params.constBegin(), params.constEnd(), cp);
  if (iter==params.constEnd())
    iter--;
  //cout << "iterpos " << iter-params.constBegin() << endl;
  return qMax(0,iter-params.constBegin());
}

BezierCurve::CurveParams BezierCurve::getCurveParam(float x) {
  //cout << "get Curve Param for " << x << endl;
  int p=getCurveParamIdx(x);
  //cout << "is Param " << p << "/" << params.size() << endl;
  return params[p];
}


