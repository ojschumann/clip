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

#ifndef MOUSEPOSITIONINFO_H
#define MOUSEPOSITIONINFO_H

#include "tools/vec3D.h"
#include "core/reflection.h"
#include <QPointF>

class QObject;

class MousePositionInfo {
public:
  MousePositionInfo() {
    valid = false;
    scatteredOk=false;
    nearestOk=false;
    lockOnNearest=false;
    projector=0;
    detQMin=0.0;
    detQMax=0.0;
  }

  bool valid;
  QPointF projectorPos;
  QPointF imagePos;
  Vec3D normal;

  bool scatteredOk;
  Vec3D scattered;

  bool nearestOk;
  bool lockOnNearest;
  double detQMin;
  double detQMax;
  Reflection nearestReflection;

  QObject* projector;
};



#endif // MOUSEPOSITIONINFO_H
