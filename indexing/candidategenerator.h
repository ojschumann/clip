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

#ifndef CANDIDATEGENERATOR_H
#define CANDIDATEGENERATOR_H

#include <QObject>
#include <QList>
#include <QReadWriteLock>

#include "tools/vec3D.h"
#include "tools/mat3D.h"
#include "config.h"

class CandidateGenerator: public QObject
{
  Q_OBJECT
public:
  struct Candidate {
    TVec3D<int> index;
    Vec3D realNormal;
    Vec3D reziprocalNormal;
    Vec3D normal;
    Candidate& spot() {
      normal = reziprocalNormal;
      return *this;
    }
    Candidate& zone() {
      normal = realNormal;
      return *this;
    }
  };

  CandidateGenerator(const Mat3D&, const Mat3D&, QObject* _parent=nullptr);
  Candidate getCandidate(int);
  QList<Candidate> getCandidateList(int until);

  void reset();
signals:
  void nextMajorIndex(int);
  void progessInfo(int);
private:
  void generateNextIndex();
  void addToGroup(const TMat3D<int>&);

  QList< TMat3D<int> > group;
  QList< Candidate > candidates;
  TVec3D<int> maxIndex;
  QReadWriteLock locker;

  Mat3D MReal;
  Mat3D MReciprocal;
};

#endif // CANDIDATEGENERATOR_H
