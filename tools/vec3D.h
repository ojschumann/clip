#ifndef __TVEC3D_H__
#define __TVEC3D_H__

#include <tools/init3D.h>

template <typename T> class TMat3D;

template<typename T> class TVec3D {
  template <typename V> friend class TMat3D;
  template <typename V> friend class TVec3D;
public:
  TVec3D();
  TVec3D(T x, T y, T z);
  TVec3D(const T _x[3]);

  TVec3D<T> operator+(const TVec3D<T> & v) const;
  TVec3D<T> operator-(const TVec3D<T> & v) const;
  TVec3D<T> operator*(const T& a) const;
  template <typename U> T operator*(const TVec3D<U>& v) const;
  TVec3D<T> operator%(const TVec3D<T> &v) const;
  TMat3D<T> operator^(const TVec3D<T> &v) const;

  bool operator==(const TVec3D<T>& v) const;
  bool operator!=(const TVec3D<T>& v) const;

  template <typename U> TVec3D<T>& convert(const TVec3D<U>& v);

  TVec3D<T>& operator+=(const TVec3D<T>& v);
  TVec3D<T>& operator-=(const TVec3D<T>& v);
  TVec3D<T>& operator*=(const T& a);

  T& operator()(int i) {return XX[i]; }
  const T& operator()(int i) const { return XX[i]; }

  T norm_sq() const;

  T x() const;
  T y() const ;
  T z() const;

  TVec3D<T> operator/(const T& a) const;
  TVec3D<T>& operator/=(const T& a);
  double norm() const;
  void normalize();
  TVec3D normalized() const;

  bool isNull() const;
  template <typename U> TVec3D<U> toType() const;

protected:
  struct KeyClass { };
  const static KeyClass Key;
  // Does not initialize
  inline TVec3D(KeyClass v) {};

  // The REAL Data
  T XX[3];

  //static T fault;
};


typedef TVec3D<double> Vec3D;



#include <tools/vec3d.cpp>

#endif
