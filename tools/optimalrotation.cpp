/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

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

#include "optimalrotation.h"

OptimalRotation::OptimalRotation()
{
  reset();
}

void OptimalRotation::reset() {
  valid = false;
  stack.zero();
}

void OptimalRotation::addVectorPair(const Vec3D &from, const Vec3D &to) {
  stack += to.normalized()^from.normalized();
  valid=false;
}

Mat3D OptimalRotation::getOptimalRotation() {
  if (!valid) {
    optRot = stack;
    Mat3D L,R;
    optRot.fastsvd(L, R);
    optRot = L*R;
    valid = true;
  }
  return optRot;
};




Mat3D VectorPairRotation(const Vec3D& from1, const Vec3D& from2, const Vec3D& to1, const Vec3D& to2) {
  Vec3D from_x = (from1.normalized()+from2.normalized()).normalized();
  Vec3D from_y = (from1.normalized()-from2.normalized()).normalized();
  Mat3D Mfrom(from_x, from_y, from_x%from_y);

  Vec3D to_x = (to1.normalized()+to2.normalized()).normalized();
  Vec3D to_y = (to1.normalized()-to2.normalized()).normalized();
  Mat3D Mto(to_x, to_y, to_x%to_y);

  return Mto * Mfrom.inverse();
}
