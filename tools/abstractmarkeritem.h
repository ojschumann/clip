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

#ifndef ABSTRACTMARKERITEM_H
#define ABSTRACTMARKERITEM_H

#include "tools/vec3D.h"

class Projector;

class AbstractMarkerItem
{
public:
  enum MarkerType {
    SpotMarker,
    ZoneMarker
  };

  AbstractMarkerItem(MarkerType t);
  virtual ~AbstractMarkerItem();

  virtual Vec3D getMarkerNormal() const = 0;

  virtual Vec3D getRationalIndex();
  virtual TVec3D<int> getIntegerIndex();

  double getIndexDeviationScore();
  double getDetectorPositionScore();
  double getAngularDeviation();

  void setMaxSearchIndex(int n);
  void setIndex(const TVec3D<int>& index);

  MarkerType getType() const { return markerType; }

  void invalidateCache();

  virtual void highlight(bool b)=0;

protected:
  virtual Vec3D normalToIndex(const Vec3D&)=0;
  void calcBestIndex();
  virtual void calcDetectorDeviation();
  virtual void calcAngularDeviation();

  MarkerType markerType;
  Vec3D rationalIndex;
  TVec3D<int> integerIndex;

  double indexDeviation;
  double detectorPositionDeviation;
  double angularDeviation;
  int maxSearchIndex;
};

class AbstractProjectorMarkerItem: public AbstractMarkerItem
{
public:
  AbstractProjectorMarkerItem(Projector* p, MarkerType t);
  virtual ~AbstractProjectorMarkerItem();
  Projector* getProjector() const { return projector; }
protected:
  virtual Vec3D normalToIndex(const Vec3D&);
  virtual void calcDetectorDeviation();
  virtual void calcAngularDeviation();
  Projector* projector;
};


#endif // ABSTRACTMARKERITEM_H
