/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

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

  T& operator()(int i) {return DATA[i]; }
  const T& operator()(int i) const { return DATA[i]; }

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
  enum Key {
    NoInit
  };
  // Does not initialize
  inline TVec3D(Key) {};

  // The REAL Data
  T DATA[3];

  //static T fault;
};


typedef TVec3D<double> Vec3D;



#include <tools/vec3D.cpp>

#endif
