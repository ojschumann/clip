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

#ifndef REFLECTION_H
#define REFLECTION_H

#include <QVector>
#include <tools/mat3D.h>
#include <tools/vec3D.h>

class  Reflection {
public:
  //Reflection(): orders() {}
  // Index
  int h;
  int k;
  int l;

  TVec3D<int> hkl() const { return TVec3D<int>(h,k,l); }
  
  //h^2+k^2+l^2
  int hklSqSum;

  // lowest order (if ==0, this reflection is not in scattering position)
  int lowestDiffOrder;

  // highest order (if ==0, this reflection is not in scattering position)
  int highestDiffOrder;

  // direct space d-Value
  double d;

  // Reziprocal lattice Vector = 1/(2*d)
  double Q;

  // Order 1 scattering Wavevectors (Q/sin(theta))
  double Qscatter;

  // All contributing orders
  QVector<int> orders;
  // Real and imaginary part of Structure factor
  //vector<double> Fr;
  //vector<double> Fi;
  
  // reziprocal direction (without rotations)
  Vec3D normalLocal;

  // reziprocal direction (with rotations)
  Vec3D normal;

  // Direction of scattered ray
  Vec3D scatteredRay;

  static QString formatOverline(int);
  static QString hkl2text(int, int, int);
  QString toHtml() const;
  QString toText() const;
};

Q_DECLARE_TYPEINFO(Reflection, Q_MOVABLE_TYPE);

#endif
