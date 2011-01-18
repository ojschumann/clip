#include "solution.h"

#include <cmath>

#include "tools/optimalrotation.h"

Solution::Solution() {
  hklDeviation = -1.0;
  indexMean = -1.0;
  indexRMS = -1.0;
}

Solution::Solution(const Solution &s) {
  *this = s;
}

Solution& Solution::operator=(const Solution& s) {
  bestRotation = s.bestRotation;
  hklDeviation = s.hklDeviation;
  indexMean = s.indexMean;
  indexRMS = s.indexRMS;
  markerIdx = s.markerIdx;
  markerScore = s.markerScore;
  return *this;
}

double Solution::hklDeviationSum() const {
  return hklDeviation;
}

double Solution::allIndexMean() const {
  if (indexMean<0) {
    double tmpIndexMean = 0.0;
    foreach (TVec3D<int> idx, markerIdx) {
      tmpIndexMean += abs(idx.x()) + abs(idx.y()) + abs(idx.z());
    }
    tmpIndexMean /= 3.0*markerIdx.size();
    const_cast<Solution*>(this)->indexMean = tmpIndexMean;
  }
  return indexMean;
}
double Solution::allIndexRMS() const {
  if (indexRMS<0) {
    double tmpIndexRMS= 0.0;
    foreach (TVec3D<int> idx, markerIdx) {
      tmpIndexRMS += idx.norm_sq();
    }
    tmpIndexRMS /= 3.0*markerIdx.size();
    tmpIndexRMS = sqrt(tmpIndexRMS);
    const_cast<Solution*>(this)->indexRMS = tmpIndexRMS;
  }
  return indexRMS;
}

int Solution_ID = qRegisterMetaType<Solution>("Solution");
