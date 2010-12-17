#ifndef MOUSEPOSITIONINFO_H
#define MOUSEPOSITIONINFO_H

#include "tools/vec3D.h"
#include "core/reflection.h"
#include <QPointF>

class MousePositionInfo {
public:
  MousePositionInfo() {
    valid = false;
    scatteredOk=false;
    nearestOk=false;
  }

  bool valid;
  QPointF projectorPos;
  QPointF imagePos;
  Vec3D normal;
  bool scatteredOk;
  Vec3D scattered;
  bool nearestOk;
  double detQMin;
  double detQMax;
  Reflection nearestReflection;
};



#endif // MOUSEPOSITIONINFO_H
