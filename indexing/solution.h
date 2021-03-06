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

#ifndef SOLUTION_H
#define SOLUTION_H

#include <QList>

#include "tools/vec3D.h"
#include "tools/mat3D.h"
#include "indexing/indexer.h"

class Solution {
public:
  Solution();
  Solution(const Solution& s);
  Solution& operator=(const Solution& s);

  Mat3D bestRotation;
  double indexDeviation;
  double indexDeviationSq;
  double indexMean;
  double indexRMS;

  // Used in sorting the solutionmodel for updating the PersistentModelIndices
  int solutionIndex;

  double allIndexRMS() const;
  double allIndexMean() const;
  double hklDeviationSum() const;
  double hklDeviationSqSum() const;

  QList< TVec3D<int> > markerIdx;
  QList<Vec3D> markerRationalIdx;

};

#include <QMetaType>
Q_DECLARE_METATYPE(Solution);

#endif // SOLUTION_H
