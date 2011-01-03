#include "solution.h"

#include "tools/optimalrotation.h"

Solution::Solution() {
}

Solution::Solution(const Solution &s) {
  *this = s;
}

Solution& Solution::operator=(const Solution& s) {
  bestRotation = s.bestRotation;
  hklDeviation = s.hklDeviation;
  markerIdx = s.markerIdx;
  markerScore = s.markerScore;

  return *this;
}

double Solution::hklDeviationSum() const {
  return hklDeviation;
}

double Solution::spatialDeviationSum() const {
  return 0;
}
double Solution::angularDeviationSum() const {
  return 0;
}

int Solution_ID = qRegisterMetaType<Solution>("Solution");
