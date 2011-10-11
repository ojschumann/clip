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

#ifndef MARKER_H
#define MARKER_H

#include "tools/abstractmarkeritem.h"
#include "tools/vec3D.h"
#include "tools/mat3D.h"


class Marker : public AbstractMarkerItem
{
public:
  Marker(const Vec3D& n, MarkerType t, int _maxSearchIndex);
  Vec3D getMarkerNormal() const;
  Vec3D getIndexNormal();
  void setMatrices(const Mat3D& SpotNormalToIndex, const Mat3D& ZoneNormalToIndex, const Mat3D& MReciprocal, const Mat3D& MReal);
protected:
  Vec3D normalToIndex(const Vec3D &);
  Vec3D markerNormal;
  Mat3D const* normalToIndexMatrix;
  Mat3D const* indexToNormalMatrix;
  virtual void highlight(bool /*b*/) {}
};

#endif // MARKER_H
