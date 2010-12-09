#include "candidategenerator.h"

#include <set>

#include "core/spacegroup.h"

using namespace std;


CandidateGenerator::CandidateGenerator(const Mat3D& _MReal, const Mat3D& _MReciprocal, QObject* parent):
    QObject(parent),
    maxIndex(0,0,0),
    MReal(_MReal),
    MReciprocal(_MReciprocal)
{
  addToGroup(TMat3D<int>(-1, 0, 0, 0,-1, 0, 0, 0,-1));
  addToGroup(TMat3D<int>( 0, 1, 0, 0, 0, 1, 1, 0, 0));
  addToGroup(TMat3D<int>( 0, 0, 1, 0, 1, 0, 1, 0, 0));
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

CandidateGenerator::Candidate CandidateGenerator::getCandidate(int n) {
  while (n>=candidates.size()) {
    generateNextIndex();
  }
  return candidates.at(n);
}

void CandidateGenerator::reset() {
  candidates.clear();
  maxIndex = TVec3D<int>(0,0,0);
}

class Vec3DOrder {
public:
  bool operator()(const TVec3D<int>& v1, const TVec3D<int>& v2) {
    for (int i=0; i<3; i++) {
      if (v1(i)<v2(i)) return true;
      if (v1(i)>v2(i)) return false;
    }
    return false;
  }
};

void CandidateGenerator::generateNextIndex() {
  maxIndex(2)++;
  for (int i=2; i--; ) {
    if (maxIndex(i+1)>maxIndex(i)) {
      maxIndex(i)++;
      maxIndex(i+1)=0;
      if (i==0) emit nextMajorIndex(maxIndex(0));
    }
  }

  set< TVec3D<int>, Vec3DOrder> vectorGroup;

  foreach(TMat3D<int> M, group) {
    vectorGroup.insert(M*maxIndex);
  }

  foreach(TVec3D<int> idx, vectorGroup) {
    Candidate cand;
    cand.index = idx;
    cand.index = idx.toType<double>().normalized();
    cand.realNormal = (MReal*idx).normalized();
    cand.reziprocalNormal = (MReciprocal*idx).normalized();

    candidates << cand;
  }


}
