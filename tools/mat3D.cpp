#ifndef __TMAT3D_CPP__
#define __TMAT3D_CPP__

#ifdef __TMAT3D_H__

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
      (*this)(i,j)=m(j+i);
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
  TMat3D<T> r(TMat3D<T>::Key);
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      r(i,j)=(*this)(i,j)+m(i,j);
  return r;
}

template <typename T> TMat3D<T> TMat3D<T>::operator-(const TMat3D<T>& m) const {
  TMat3D<T> r(TMat3D<T>::Key);
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      r(i,j)=(*this)(i,j)-m(i,j);
  return r;
}

template <typename T> TMat3D<T> TMat3D<T>::operator*(T a) const {
  TMat3D<T> r(TMat3D<T>::Key);
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      r(i,j)=(*this)(i,j)*a;
  return r;
}

template <typename T> template <typename U> TVec3D<T> TMat3D<T>::operator*(const TVec3D<U>& v) const {
    TVec3D<T> r(TVec3D<T>::Key);
    for (int i=0; i<3; i++) {
      T tmp = (*this)(i,0)*v(0);
      for (int j=1; j<3; j++)
        tmp += (*this)(i,j)*v(j);
      r(i) = tmp;
    }
    return r;
}

template <typename T> TMat3D<T> TMat3D<T>::operator*(const TMat3D<T>& m) const {
  TMat3D<T> r(TMat3D<T>::Key);
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


template <typename T> bool TMat3D<T>::operator==(const TMat3D<T>& m) {
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      if (!((*this)(i,j)==m(i,j)))
        return false;
    }
  }
  return true;
}

template <typename T> TVec3D<T> TMat3D<T>::operator[](int i) const {
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
  TMat3D<T> r(TMat3D<T>::Key);
  for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) {
          r(i,j)=(*this)(j,i);
      }
  }
  return r;
}

template <typename T> TMat3D<T> TMat3D<T>::inverse() const {
  TMat3D<T> r(TMat3D<T>::Key);
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
    for (int i=0; i<3-n; i++) u[2-i]=(*this)(2-i,n);
    u[n]+=u.norm();
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
    for (int i=0; i<3-n; i++) u[i]=(*this)(i,2-n);
    u[2-n]+=u.norm();
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


template <typename T> void TMat3D<T>::svd(TMat3D<T>& L, TMat3D<T>& R) {

    upperBidiagonal(L,R);
    L.transpose();
    
    int maxLoops=512;
    double sumDiag=0.0;
    double sumOffdiag=0.0;
        
    do {

        // Givens rotation
        // TODO: Could be done without the whole matrix multiplications!!! See GSL
        for (int n=0; n<2; n++) {
            double c,s;
            givens((*this)(0,n),(*this)(0,n+1), c, s);
            TMat3D G1;
            G1(n,n)=c;
            G1(n+1,n+1)=c;
            G1(n+1,n)=-s;
            G1(n,n+1)=s;
            
            (*this)*=G1;
            R*=G1;
            
            givens((*this)(n,n), (*this)(n+1,n), c, s);
            TMat3D G2;
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
};    
    

#endif
#endif
