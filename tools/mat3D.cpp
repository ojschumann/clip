#ifndef __TMAT3D_CPP__
#define __TMAT3D_CPP__

#include "tools/mat3D.h"
#include "tools/vec3D.h"

//#ifdef __TMAT3D_H__

#include <cmath>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <algorithm>
#include <type_traits>


using namespace std;

template <typename T> TMat3D<T>::TMat3D() {
  zero();
  for (int i=0; i<3; i++)
    (*this)(i,i)=InitatorValues<T>::One();
}

template <typename T> TMat3D<T>::TMat3D(const T& val) {
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
    (*this)(i,j)=val;
}

template <typename T> TMat3D<T>::TMat3D(const TVec3D<T>& v1,const TVec3D<T>& v2,const TVec3D<T>& v3) {
  for (int i=0; i<3; i++) {
    (*this)(i,0)=v1(i);
    (*this)(i,1)=v2(i);
    (*this)(i,2)=v3(i);
  }
}


template <typename T> TMat3D<T>::TMat3D(TVec3D<T> axis, T angle) {
  // from http://www.euclideanspace.com/maths/algebra/matrix/orthogonal/rotation/
  
  double s = sin(angle);
  double c = cos(angle);
  T x = axis.x();
  T y = axis.y();
  T z = axis.z();

  (*this)(0,0)=1.0+(1.0-c)*(x*x-1.0);
  (*this)(1,1)=1.0+(1.0-c)*(y*y-1.0);
  (*this)(2,2)=1.0+(1.0-c)*(z*z-1.0);

  (*this)(0,1)=-z*s+(1.0-c)*x*y;
  (*this)(0,2)= y*s+(1.0-c)*x*z;

  (*this)(1,0)= z*s+(1.0-c)*x*y;
  (*this)(1,2)=-x*s+(1.0-c)*y*z;

  (*this)(2,0)=-y*s+(1.0-c)*x*z;
  (*this)(2,1)= x*s+(1.0-c)*y*z;
}


template <typename T> TMat3D<T>::TMat3D(vector<T> m) {
  for (int i=0; i<3; i++) {
    for (int j=0; j<9; j+=3)
      (*this)(i,j)=m[j+i];
  }
}

template <typename T> TMat3D<T>::TMat3D(T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33) {
  (*this)(0,0) = m11;
  (*this)(0,1) = m12;
  (*this)(0,2) = m13;
  (*this)(1,0) = m21;
  (*this)(1,1) = m22;
  (*this)(1,2) = m23;
  (*this)(2,0) = m31;
  (*this)(2,1) = m32;
  (*this)(2,2) = m33;
}


template <typename T> void TMat3D<T>::zero() {
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      (*this)(i,j)=InitatorValues<T>::Zero();
    }
  }
}


template <typename T> TMat3D<T> TMat3D<T>::operator+(const TMat3D<T>& m) const {
  TMat3D<T> r(TMat3D<T>::NoInit);
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      r(i,j)=(*this)(i,j)+m(i,j);
  return r;
}

template <typename T> TMat3D<T> TMat3D<T>::operator-(const TMat3D<T>& m) const {
  TMat3D<T> r(TMat3D<T>::NoInit);
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      r(i,j)=(*this)(i,j)-m(i,j);
  return r;
}

template <typename T> TMat3D<T> TMat3D<T>::operator*(T a) const {
  TMat3D<T> r(TMat3D<T>::NoInit);
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      r(i,j)=(*this)(i,j)*a;
  return r;
}

template <typename T> template <typename U> TVec3D<T> TMat3D<T>::operator*(const TVec3D<U>& v) const {
  TVec3D<T> r(TVec3D<T>::NoInit);
  for (int i=0; i<3; i++) {
    T tmp = (*this)(i,0)*v(0);
    for (int j=1; j<3; j++)
      tmp += (*this)(i,j)*v(j);
    r(i) = tmp;
  }
  return r;
}

template <typename T> TMat3D<T> TMat3D<T>::operator*(const TMat3D<T>& m) const {
  TMat3D<T> r(TMat3D<T>::NoInit);
  for (int j=0; j<3; j++) {
    for (int i=0; i<3; i++) {
      T tmp = (*this)(i,0)*m(0,j);
      for (int k=1; k<3; k++)
        tmp += (*this)(i,k)*m(k,j);
      r(i,j) = tmp;
    }
  }
  return r;
}


template <typename T> TMat3D<T>& TMat3D<T>::operator*=(const TMat3D<T>& m) {
  TMat3D tmp(*this);
  for (int j=0; j<3; j++) {
    for (int i=0; i<3; i++) {
      T tmp2=tmp(i,0)*m(0,j);
      for (int k=1; k<3; k++)
        tmp2+=tmp(i,k)*m(k,j);
      (*this)(i,j) = tmp2;
    }
  }
  return *this;
}

template <typename T> TMat3D<T>& TMat3D<T>::lmult(const TMat3D<T>& m) {
  TMat3D tmp(*this);
  for (int j=0; j<3; j++) {
    for (int i=0; i<3; i++) {
      T tmp2=m(i,0)*tmp(0,j);
      for (int k=1; k<3; k++)
        tmp2+=m(i,k)*tmp(k,j);
      (*this)(i,j) = tmp2;
    }
  }
  return *this;
}

template <typename T> TMat3D<T>& TMat3D<T>::operator+=(const TMat3D<T>& m) {
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      (*this)(i,j)+=m(i,j);
  return *this;
}

template <typename T> TMat3D<T>& TMat3D<T>::operator-=(const TMat3D<T>& m) {
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      (*this)(i,j)-=m(i,j);
  return *this;
}

template <typename T> TMat3D<T>& TMat3D<T>::operator*=(T a) {
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      (*this)(i,j)*=a;
  return *this;
}


template <typename T> bool TMat3D<T>::operator==(const TMat3D<T>& m) const {
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      if (!((*this)(i,j)==m(i,j)))
        return false;
    }
  }
  return true;
}

template <typename T> TVec3D<T> TMat3D<T>::operator()(int i) const {
  if (i<3) 
    return TVec3D<T>((*this)(i,0), (*this)(i,1), (*this)(i,2));
  return TVec3D<T>();
}


template <typename T> T TMat3D<T>::at(int i, int j) const {
  if ((i<3) && (j<3)) 
    return (*this)(i,j);
  return InitatorValues<T>::One();
}

template <typename T> TMat3D<T> TMat3D<T>::orthogonalize() const {
  TMat3D C=(TMat3D()*3.0-((*this)*transposed()))*0.5;
  return C*(*this);
}


template <typename T> void TMat3D<T>::transpose() {
  for (int i=0; i<2; i++)
    for (int j=i+1; j<3; j++)
      std::swap((*this)(i,j), (*this)(j,i));
}


template <typename T> TMat3D<T> TMat3D<T>::transposed() const {
  TMat3D<T> r(TMat3D<T>::NoInit);
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      r(i,j)=(*this)(j,i);
    }
  }
  return r;
}

template <typename T> TMat3D<T> TMat3D<T>::inverse() const {
  TMat3D<T> r(TMat3D<T>::NoInit);
  T d=this->det();
  if (d!=InitatorValues<T>::Zero()) {
    d=1.0/d;
    r(0,0)=d*((*this)(1,1)*(*this)(2,2)-(*this)(1,2)*(*this)(2,1));
    r(0,1)=d*((*this)(2,1)*(*this)(0,2)-(*this)(2,2)*(*this)(0,1));
    r(0,2)=d*((*this)(0,1)*(*this)(1,2)-(*this)(0,2)*(*this)(1,1));
    r(1,0)=d*((*this)(1,2)*(*this)(2,0)-(*this)(1,0)*(*this)(2,2));
    r(1,1)=d*((*this)(2,2)*(*this)(0,0)-(*this)(2,0)*(*this)(0,2));
    r(1,2)=d*((*this)(0,2)*(*this)(1,0)-(*this)(0,0)*(*this)(1,2));
    r(2,0)=d*((*this)(1,0)*(*this)(2,1)-(*this)(1,1)*(*this)(2,0));
    r(2,1)=d*((*this)(2,0)*(*this)(0,1)-(*this)(2,1)*(*this)(0,0));
    r(2,2)=d*((*this)(0,0)*(*this)(1,1)-(*this)(0,1)*(*this)(1,0));
  }
  return r;
}


template <typename T> T TMat3D<T>::sqSum() const {
  T s=InitatorValues<T>::Zero();
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      s+=(*this)(i,j)*(*this)(i,j);
  return s;
}

template <typename T> T TMat3D<T>::det() const {
  T d=(*this)(0,0)*(*this)(1,1)*(*this)(2,2);
  d+=(*this)(1,0)*(*this)(2,1)*(*this)(0,2);
  d+=(*this)(2,0)*(*this)(0,1)*(*this)(1,2);
  d-=(*this)(0,0)*(*this)(1,2)*(*this)(2,1);
  d-=(*this)(1,0)*(*this)(2,2)*(*this)(0,1);
  d-=(*this)(2,0)*(*this)(0,2)*(*this)(1,1);
  return d;
}

template <typename T> TMat3D<T> TMat3D<T>::QR() {
  TMat3D<T> Q;
  for (int n=0; n<2; n++) {
    TVec3D<T> u;
    for (int i=0; i<3-n; i++) u(2-i)=(*this)(2-i,n);
    u(n)+=u.norm();
    u.normalize();
    TMat3D<T> Qt(u^u);
    Qt*=2.0;
    for (int i=0; i<3; i++) Qt(i,i)-=InitatorValues<T>::One();
    lmult(Qt);
    Q*=Qt;
  }
  return Q;
}

template <typename T> TMat3D<T> TMat3D<T>::QL() {
  TMat3D<T> Q;
  for (int n=0; n<2; n++) {
    TVec3D<T> u;
    for (int i=0; i<3-n; i++) u(i)=(*this)(i,2-n);
    u(2-n)+=u.norm();
    u.normalize();
    TMat3D<T> Qt(u^u);
    Qt*=2.0;
    for (int i=0; i<3; i++) Qt(i,i)-=InitatorValues<T>::One();
    lmult(Qt);
    Q*=Qt;
  }
  return Q;
}

// L and R need to be unit matrices
template <typename T> void TMat3D<T>::upperBidiagonal(TMat3D<T>& L, TMat3D<T>& R) {
  for (int n=0; n<2; n++) {
    TVec3D<T> u;
    for (int i=0; i<3-n; i++) u(2-i)=(*this)(2-i,n);
    u(n)+=u.norm();
    u.normalize();
    TMat3D<T> Tx(u^u);
    Tx*=2.0;
    for (int i=0; i<3; i++) Tx(i,i)-=InitatorValues<T>::One();
    lmult(Tx);
    L*=Tx;

    if (n<1) {
      for (int i=0; i<3; i++) u(i)=(i<=n)?InitatorValues<T>::Zero():(*this)(n,i);
      u(n+1)+=u.norm();
      u.normalize();
      Tx=u^u;
      Tx*=2.0;
      for (int i=0; i<3; i++) Tx(i,i)-=InitatorValues<T>::One();
      (*this)*=Tx;
      R.lmult(Tx);
    }
  }
}



template <typename T> void TMat3D<T>::givens(double a, double b, double &c, double &s) {
  if (b==0.0) {
    c=1.0;
    s=0.0;
  } else if (fabs(a)>fabs(b)) {
    double t=-b/a;
    c=1.0/sqrt(1.0+t*t);
    s=c*t;
  } else {
    double t=-a/b;
    s=1.0/sqrt(1.0+t*t);
    c=t*s;
  }
}

template <typename T> void MACRO(T& a, T& b, T& c, T& s) {
  double _a = fabs(a);
  double _b = fabs(b);
  double lambda;
  if (_a>_b) {
    lambda = _b/_a;
    lambda = _a*sqrt(1.0+lambda*lambda);
  } else {
    lambda = _a/_b;
    lambda = _b*sqrt(1.0+lambda*lambda);
  }
  //double lambda  = hypot(_a, _b);
  //double lambda  = sqrt(_a*_a+_b*_b);
  c = a/lambda;
  s = b/lambda;
  a = lambda;
  b = 0.0;
}


template <typename T> int TMat3D<T>::fastsvd(TMat3D<T>& L, TMat3D<T>& R) {

  upperBidiagonal(L,R);

  if ((L*R).det()<0) cout << "deterror on upperBidiagonal" << endl;

  int maxLoops=512;
  double sumDiag=0.0;
  double sumOffdiag=0.0;

  do {
    T c,s;
    T a, b;

    MACRO((*this)(0,0), (*this)(0,1), c, s);
    /*lambda = hypot((*this)(0,0), (*this)(0,1));
    c = (*this)(0,0)/lambda;
    s = (*this)(0,1)/lambda;
    (*this)(0,0)=lambda;
    (*this)(0,1)=0.0;*/
    (*this)(1,0) = (*this)(1,1)*s;
    (*this)(1,1) *= c;
    for (int i=0; i<3; i++) {
      a = R(0, i);
      b = R(1, i);
      R(0, i)=a*c+b*s;
      R(1, i)=b*c-a*s;
    }

    MACRO((*this)(0,0), (*this)(1,0), c, s);
    for (int i=1; i<3; i++) {
      (*this)(0,i)=s*(*this)(1,i);
      (*this)(1,i)*=c;
    }
    for (int i=0; i<3; i++) {
      a = L(i, 0);
      b = L(i, 1);
      L(i, 0)=a*c+b*s;
      L(i, 1)=b*c-a*s;
    }


    MACRO((*this)(0,1), (*this)(0,2), c, s);
    a = (*this)(1,1)*c+(*this)(1,2)*s;
    b = (*this)(1,2)*c-(*this)(1,1)*s;
    (*this)(1,1) = a;
    (*this)(1,2) = b;
    (*this)(2,1)=s*(*this)(2,2);
    (*this)(2,2)*=c;
    for (int i=0; i<3; i++) {
      a = R(1, i);
      b = R(2, i);
      R(1, i)=a*c+b*s;
      R(2, i)=b*c-a*s;
    }

    MACRO((*this)(1,1), (*this)(2,1), c, s);
    a = (*this)(1,2)*c+(*this)(2,2)*s;
    b = (*this)(2,2)*c-(*this)(1,2)*s;
    (*this)(1,2) = a;
    (*this)(2,2) = b;
    for (int i=0; i<3; i++) {
      a = L(i, 1);
      b = L(i, 2);
      L(i, 1)=a*c+b*s;
      L(i, 2)=b*c-a*s;
    }

    sumDiag=0.0;
    sumOffdiag=0.0;
    for (int i=0; i<3; i++) sumDiag+=fabs((*this)(i,i));
    for (int i=0; i<2; i++) sumOffdiag+=fabs((*this)(i,i+1));

  } while (maxLoops-- and sumDiag<1e20*sumOffdiag);
  TMat3D<T> XX = L * (*this) * R;
  return maxLoops;
}



template <typename T> int TMat3D<T>::svd(TMat3D<T>& L, TMat3D<T>& R) {
  upperBidiagonal(L,R);
  L.transpose();

  int maxLoops=512;
  double sumDiag=0.0;
  double sumOffdiag=0.0;

  do {
    for (int n=0; n<2; n++) {
      double s,c;
      givens((*this)(0,n),(*this)(0,n+1), c, s);
      TMat3D<T> G1;
      G1(n,n)=c;
      G1(n+1,n+1)=c;
      G1(n+1,n)=-s;
      G1(n,n+1)= s;

      (*this)*=G1;
      R*=G1;

      givens((*this)(n,n), (*this)(n+1,n), c, s);
      TMat3D<T> G2;
      G2(n,n)=c;
      G2(n+1,n+1)=c;
      G2(n,n+1)=-s;
      G2(n+1,n)=s;

      this->lmult(G2);
      L.lmult(G2);
    }

    // Make singular values positive!
    TMat3D<T> Tmp;
    for (int n=0; n<3; n++)
      Tmp(n,n)=((*this)(n,n)<0.0)?-1.0:1.0;
    this->lmult(Tmp);
    L.lmult(Tmp);

    sumDiag=0.0;
    sumOffdiag=0.0;
    for (int i=0; i<3; i++) sumDiag+=fabs((*this)(i,i));
    for (int i=0; i<2; i++) sumOffdiag+=fabs((*this)(i,i+1));

  } while (maxLoops-- and sumDiag<1e20*sumOffdiag);
  L.transpose();
  R.transpose();
  return maxLoops;
};    

template <typename T> template <typename U> TMat3D<U> TMat3D<T>::toType() {
  return TMat3D<U>(static_cast<U>((*this)(0,0)),
              static_cast<U>((*this)(0,1)),
              static_cast<U>((*this)(0,2)),
              static_cast<U>((*this)(1,0)),
              static_cast<U>((*this)(1,1)),
              static_cast<U>((*this)(1,2)),
              static_cast<U>((*this)(2,0)),
              static_cast<U>((*this)(2,1)),
              static_cast<U>((*this)(2,2)));
}

template class TMat3D<double>;
template class TMat3D<int>;
template TMat3D<double> TMat3D<int>::toType();
template TVec3D<double> TMat3D<double>::operator*(const TVec3D<double>& v) const;
template TVec3D<int> TMat3D<int>::operator*(const TVec3D<int>& v) const;
template TVec3D<double> TMat3D<double>::operator*(const TVec3D<int>& v) const;
//#endif
#endif
