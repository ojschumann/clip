#ifndef OPTIMALROTATION_H
#define OPTIMALROTATION_H

#include <tools/vec3D.h>
#include <tools/mat3D.h>

class OptimalRotation
{
public:
  OptimalRotation();
  void reset();
  void addVectorPair(const Vec3D& from, const Vec3D& to);
  Mat3D getOptimalRotation();

private:
  Mat3D stack;
  Mat3D optRot;
  bool valid;
};

Mat3D VectorPairRotation(const Vec3D& from1, const Vec3D& from2, const Vec3D& to1, const Vec3D& to2);

#endif // OPTIMALROTATION_H
