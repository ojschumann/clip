#include "solution.h"

double SolutionItem::angularDeviation() const {
  return 180.0*M_1_PI*fabs(acos(rotatedMarker*latticeVector));
}

double SolutionItem::spatialDeviation() const {
  return fabs((rotatedMarker-latticeVector).norm());
}

double SolutionItem::hklDeviation() const {
  return fabs(rationalHkl[0]-h)+fabs(rationalHkl[1]-k)+fabs(rationalHkl[2]-l);
}

double Solution::angularDeviationSum() const {
  double s=0.0;
  for (unsigned int n=items.size(); n--; )
    s+=items.at(n).angularDeviation();
  return s;
}

double Solution::spatialDeviationSum() const {
  double s=0.0;
  for (unsigned int n=items.size(); n--; )
    s+=items.at(n).spatialDeviation();
  return s;
}

double Solution::hklDeviationSum() const {
  double s=0.0;
  for (unsigned int n=items.size(); n--; )
    s+=items.at(n).hklDeviation();
  return s;
}


