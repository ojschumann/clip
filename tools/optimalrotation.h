#ifndef OPTIMALROTATION_H
#define OPTIMALROTATION_H

#include <tools/vec3D.h>
#include <tools/mat3D.h>

class OptimalRotation
{
public:
  OptimalRotation();
  void reset();
  void addVectorPair(const Vec3D&, const Vec3D&);
  Mat3D getOptimalRotation();

private:
  Mat3D stack;
  Mat3D optRot;
  bool valid;
};

#endif // OPTIMALROTATION_H
