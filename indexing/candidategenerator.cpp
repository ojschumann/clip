#include "candidategenerator.h"

#include "core/spacegroup.h"

CandidateGenerator::CandidateGenerator()
{
  addToGroup(Mat3D(-1, 0, 0, 0,-1, 0, 0, 0,-1));
  addToGroup(Mat3D( 0, 1, 0, 0, 0, 1, 1, 0, 0));
  addToGroup(Mat3D( 0, 0, 1, 0, 1, 0, 1, 0, 0));
}

void CandidateGenerator::addToGroup(const TMat3D<int> &e) {
  if (!group.contains(e)) {
    group << e;
    for (int i=0; i<group.size(); i++) {
      addToGroup(group.at(i)*e);
      addToGroup(e*group.at(i));
    }
  }
}
