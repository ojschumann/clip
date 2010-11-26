#ifndef __TMAT3D_H__
#define __TMAT3D_H__


#include <vector>
#include <tools/init3D.h>
using namespace std;


template <typename T> class TVec3D;



template <typename T> class TMat3D {
  friend class TVec3D<T>;
public:
  TMat3D();
  TMat3D(const TVec3D<T>& v1, const TVec3D<T>& v2, const TVec3D<T>& v3);
  TMat3D(vector<T> M);
  TMat3D(TVec3D<T> axis, T angle);
  TMat3D(T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33);
  TMat3D<T> operator*(T a) const;
  template <typename U> TVec3D<T> operator*(const TVec3D<U>& v) const;
  TMat3D<T> operator*(const TMat3D<T>& m) const;
  
  TMat3D<T> operator+(const TMat3D<T>& m) const;
  TMat3D<T> operator-(const TMat3D<T>& m) const;

  TMat3D<T>& operator*=(const TMat3D<T>& m);
  TMat3D<T>& operator*=(T a);
  TMat3D<T>& operator+=(const TMat3D<T>& m);
  TMat3D<T>& operator-=(const TMat3D<T>& m);
  TMat3D<T>& lmult(const TMat3D<T>& m);

  bool operator==(const TMat3D<T>& m);

  TVec3D<T> operator[](int i) const;
  T& operator()(int i, int j) { return DATA[i+j+(j<<1)]; };
  const T& operator()(int i, int j) const { return DATA[i+j+(j<<1)]; }
  T at(int i, int j) const;

  TMat3D<T> transposed() const;
  void transpose();
  T sqSum() const;
  T det() const;
  void zero();

  template <typename U> TMat3D<U> toType();
  
  TMat3D<T> orthogonalize() const;
  TMat3D<T> inverse() const;

  // returns Q, Matrix will be overwritten
  TMat3D<T> QR();
  TMat3D<T> QL();
  void upperBidiagonal(TMat3D<T>& L, TMat3D<T>& R);
  void svd(TMat3D<T>& L, TMat3D<T>& R);

protected:
  struct KeyClass {};
  static const KeyClass Key;
  inline TMat3D(KeyClass ) {};

  inline void givens(double a, double b, double &c, double &s);

  T DATA[9];
};


typedef TMat3D<double> Mat3D;


#include<tools/mat3d.cpp>

#endif
