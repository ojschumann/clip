/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

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

#include "marker.h"
#include "indexer.h"

Marker::Marker(const Vec3D &n, MarkerType t, int _maxSearchIndex):
    AbstractMarkerItem(t),
    markerNormal(n),
    normalToIndexMatrix(0)
{
  setMaxSearchIndex(_maxSearchIndex);
}


Vec3D Marker::getMarkerNormal() const {
  return markerNormal;
}

Vec3D Marker::normalToIndex(const Vec3D &n) {
  return *normalToIndexMatrix * n;
}

Vec3D Marker::getIndexNormal() {
  Vec3D v = getIntegerIndex().toType<double>();
  v = *indexToNormalMatrix * v;
  v.normalize();
  return v;
  return (*indexToNormalMatrix * getIntegerIndex().toType<double>()).normalized();
}

void Marker::setMatrices(const Mat3D &SpotNormalToIndex, const Mat3D &ZoneNormalToIndex, const Mat3D &MReciprocal, const Mat3D &MReal) {
  if (markerType==SpotMarker) {
    normalToIndexMatrix = &SpotNormalToIndex;
    indexToNormalMatrix = &MReciprocal;
  } else {
    normalToIndexMatrix = &ZoneNormalToIndex;
    indexToNormalMatrix = &MReal;
  }
}
