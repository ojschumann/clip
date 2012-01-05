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

#include "solution.h"

#include <cmath>

#include "tools/optimalrotation.h"

Solution::Solution() {
  indexDeviation = -1.0;
  indexDeviationSq = -1.0;
  indexMean = -1.0;
  indexRMS = -1.0;
}

Solution::Solution(const Solution &s) {
  *this = s;
}

Solution& Solution::operator=(const Solution& s) {
  bestRotation = s.bestRotation;
  indexDeviation = s.indexDeviation;
  indexDeviationSq = s.indexDeviationSq;
  indexMean = s.indexMean;
  indexRMS = s.indexRMS;
  solutionIndex = s.solutionIndex;
  markerIdx = s.markerIdx;
  markerRationalIdx = s.markerRationalIdx;

  return *this;
}

double Solution::hklDeviationSum() const {
  if (indexDeviation<0) {
    double tmpIndexDeviation = 0.0;
    for (int n=0; n<markerIdx.size(); n++) {
      tmpIndexDeviation += (markerRationalIdx.at(n) - markerIdx.at(n).toType<double>()).norm();
    }
    tmpIndexDeviation /= markerIdx.size();
    const_cast<Solution*>(this)->indexDeviation = tmpIndexDeviation;
  }

  return indexDeviation;
}

double Solution::hklDeviationSqSum() const {
  if (indexDeviationSq<0) {
    double tmpIndexDeviation = 0.0;
    for (int n=0; n<markerIdx.size(); n++) {
      tmpIndexDeviation += (markerRationalIdx.at(n) - markerIdx.at(n).toType<double>()).norm_sq();
    }
    tmpIndexDeviation /= 3.0*markerIdx.size();
    const_cast<Solution*>(this)->indexDeviationSq = sqrt(tmpIndexDeviation);
  }

  return indexDeviationSq;
}

double Solution::allIndexMean() const {
  if (indexMean<0) {
    double tmpIndexMean = 0.0;
    foreach (Vec3D idx, markerRationalIdx) {
      tmpIndexMean += qAbs(idx.x()) + qAbs(idx.y()) + qAbs(idx.z());
    }
    tmpIndexMean /= 3.0*markerIdx.size();
    const_cast<Solution*>(this)->indexMean = tmpIndexMean;
  }
  return indexMean;
}
double Solution::allIndexRMS() const {
  if (indexRMS<0) {
    double tmpIndexRMS= 0.0;
    foreach (Vec3D idx, markerRationalIdx) {
      tmpIndexRMS += idx.norm_sq();
    }
    tmpIndexRMS /= 3.0*markerIdx.size();
    tmpIndexRMS = sqrt(tmpIndexRMS);
    const_cast<Solution*>(this)->indexRMS = tmpIndexRMS;
  }
  return indexRMS;
}

int Solution_ID = qRegisterMetaType<Solution>("Solution");
int QListSolution_ID = qRegisterMetaType< QList<Solution> >("QList<Solution>");
