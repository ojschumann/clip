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

#include "candidategenerator.h"

#include <set>
 
 

#include "core/spacegroup.h"
#include "tools/tools.h"

using namespace std;


CandidateGenerator::CandidateGenerator(const Mat3D& _MReal, const Mat3D& _MReciprocal, QObject* parent):
    QObject(parent),
    maxIndex(0,0,0),
    MReal(_MReal),
    MReciprocal(_MReciprocal)
{
  addToGroup(TMat3D<int>(-1, 0, 0, 0, 1, 0, 0, 0, 1));  // m
  addToGroup(TMat3D<int>( 0, 1, 0, 0, 0, 1, 1, 0, 0));  // 3
  addToGroup(TMat3D<int>(-1, 0, 0, 0, 0,-1, 0,-1, 0));  // m
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
  locker.lockForRead();
  if (n>=candidates.size()) {
    locker.unlock();
    locker.lockForWrite();
    while (n>=candidates.size()) {
      generateNextIndex();
    }
    locker.unlock();
    locker.lockForRead();
  }
  Candidate c = candidates.at(n);
  locker.unlock();
  return c;
}

QList<CandidateGenerator::Candidate> CandidateGenerator::getCandidateList(int n) {
  locker.lockForRead();
  if (n>=candidates.size()) {
    locker.unlock();
    locker.lockForWrite();
    while (n>=candidates.size()) {
      generateNextIndex();
    }
    locker.unlock();
    locker.lockForRead();
  }
  QList<Candidate> list;
  for (int i=0; i<n; i++) {
    list << candidates.at(i);
  }
  locker.unlock();
  return list;
}

void CandidateGenerator::reset() {
  locker.lockForWrite();
  candidates.clear();
  maxIndex = TVec3D<int>(0,0,0);
  locker.unlock();
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
  emit progessInfo((maxIndex(1)+1)*maxIndex(1)/2+maxIndex(2));

  if (ggt(maxIndex(0), ggt(maxIndex(1), maxIndex(2)))==1) {
    set< TVec3D<int>, Vec3DOrder> vectorGroup;

    foreach(TMat3D<int> M, group) {
      vectorGroup.insert(M*maxIndex);
    }

    foreach(TVec3D<int> idx, vectorGroup) {
      Candidate cand;
      cand.index = idx;
      cand.realNormal = (MReal*idx).normalized();
      cand.reziprocalNormal = (MReciprocal*idx).normalized();

      candidates << cand;
    }
  }

}
