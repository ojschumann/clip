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

#ifndef __TVEC3D_CPP__
#define __TVEC3D_CPP__

#ifdef __TVEC3D_H__


#include <cmath>
#include <stdio.h>


using namespace std;

template <typename T> TVec3D<T>::TVec3D(){
  for (int i=0; i<3; i++) (*this)(i) = InitatorValues<T>::Zero();
}

template <typename T>  TVec3D<T>::TVec3D(T _x, T _y, T _z) {
  (*this)(0)=_x;
  (*this)(1)=_y;
  (*this)(2)=_z;
}

template <typename T> TVec3D<T>::TVec3D(const T _x[3]) {
  for (int i=0; i<3; i++) (*this)(i) = _x[i];
}


template <typename T> TVec3D<T> TVec3D<T>::operator+(const TVec3D<T>& v) const{
  TVec3D<T> r(TVec3D<T>::NoInit);
  for (int i=0; i<3; i++) r(i) = (*this)(i)+v(i);
  return r;
}

template <typename T> TVec3D<T> TVec3D<T>::operator-(const TVec3D<T>& v) const {
  TVec3D<T> r(TVec3D<T>::NoInit);
  for (int i=0; i<3; i++) r(i) = (*this)(i)-v(i);
  return r;
}

template <typename T> template <typename U> T TVec3D<T>::operator*(const TVec3D<U>& v) const {
  T r = (*this)(0)*v(0);
  for (int i=1; i<3; i++) r+=(*this)(i)*v(i);
  return r;
}



template <typename T> TVec3D<T> TVec3D<T>::operator*(const T& a) const {
  TVec3D<T> r(TVec3D<T>::NoInit);
  for (int i=0; i<3; i++) r(i) = (*this)(i) * a;
  return r;
}

template <typename T> TVec3D<T> TVec3D<T>::operator%(const TVec3D<T> &v) const {
  TVec3D<T> r(TVec3D<T>::NoInit);
  r(0)=(*this)(1)*v(2)-(*this)(2)*v(1);
  r(1)=(*this)(2)*v(0)-(*this)(0)*v(2);
  r(2)=(*this)(0)*v(1)-(*this)(1)*v(0);
  return r;
}

template <typename T> TMat3D<T> TVec3D<T>::operator^(const TVec3D<T> &v) const {
  TMat3D<T> M(TMat3D<T>::NoInit);
  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      M(i,j)=(*this)(i)*v(j);
    }
  }
  return M;
}

template <typename T> TVec3D<T> TVec3D<T>::operator/(const T& a)  const {
  if (a==0) return TVec3D<T>(*this);
  TVec3D<T> r(TVec3D<T>::NoInit);
  for (int i=0; i<3; i++) r(i) = (*this)(i) / a;
  return r;
}


template <typename T> TVec3D<T>& TVec3D<T>::operator+=(const TVec3D<T>& v) {
  for (int i=0; i<3; i++) (*this)(i) += v(i);
  return *this;
}

template <typename T> TVec3D<T>& TVec3D<T>::operator-=(const TVec3D<T>& v){
  for (int i=0; i<3; i++) (*this)(i) -= v(i);
  return *this;
}

template <typename T> TVec3D<T>& TVec3D<T>::operator*=(const T& a){
  for (int i=0; i<3; i++) (*this)(i) *= a;
  return *this;
}

template <typename T> TVec3D<T>& TVec3D<T>::operator/=(const T& a) {
  if (a!=0)
    for (int i=0; i<3; i++) (*this)(i) /= a;
  return *this;
}

template <typename T> template <typename U> TVec3D<T>& TVec3D<T>::convert(const TVec3D<U>& v) {
  for (int i=0; i<3; i++)
    (*this)(i)=v(i);
  return *this;
}


template <typename T> double TVec3D<T>::norm()  const {
  return sqrt((*this)(0)*(*this)(0)+(*this)(1)*(*this)(1)+(*this)(2)*(*this)(2));
}

template <typename T> T TVec3D<T>::norm_sq()  const {
  return (*this)(0)*(*this)(0)+(*this)(1)*(*this)(1)+(*this)(2)*(*this)(2);
}

template <typename T> void TVec3D<T>::normalize() {
  *this /= norm();
}

template <typename T> TVec3D<T> TVec3D<T>::normalized() const {
  TVec3D<T> r(*this);
  r.normalize();
  return r;
}

template <typename T> T TVec3D<T>::x()  const {
  return (*this)(0);
}

template <typename T> T TVec3D<T>::y()  const {
  return (*this)(1);
}

template <typename T> T TVec3D<T>::z()  const {
  return (*this)(2);
}

template <typename T> bool TVec3D<T>::operator==(const TVec3D<T>& v)  const {
  return ((*this)(0)==v(0)) && ((*this)(1)==v(1)) && ((*this)(2)==v(2));
}

template <typename T> bool TVec3D<T>::operator!=(const TVec3D<T>& v)  const {
  return ((*this)(0)!=v(0)) || ((*this)(1)!=v(1)) || ((*this)(2)!=v(2));
}

template <typename T> bool TVec3D<T>::isNull()  const {
  return ((*this)(0)==InitatorValues<T>::Zero()) && ((*this)(1)==InitatorValues<T>::Zero()) && ((*this)(2)==InitatorValues<T>::Zero());
}

template <typename T> template <typename U> TVec3D<U> TVec3D<T>::toType() const {
  return TVec3D<U>(static_cast<U>((*this)(0)),
              static_cast<U>((*this)(1)),
              static_cast<U>((*this)(2)));
}
//template <typename T> T TVec3D<T>::fault=InitatorValues<T>::Zero;

#endif
#endif
