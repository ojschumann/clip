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
  TMat3D(const T&);
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

  bool operator==(const TMat3D<T>& m) const;

  TVec3D<T> operator()(int i) const;
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
  int svd(TMat3D<T>& L, TMat3D<T>& R);
  int fastsvd(TMat3D<T>& L, TMat3D<T>& R);

protected:
  enum Key {
    NoInit
  };

  inline TMat3D(Key) {}

  inline void givens(double a, double b, double &c, double &s);

  T DATA[9];
};


typedef TMat3D<double> Mat3D;


//#include<tools/mat3d.cpp>

#endif
