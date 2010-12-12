#include "optimalrotation.h"

OptimalRotation::OptimalRotation()
{
  reset();
}

void OptimalRotation::reset() {
  valid = false;
  stack.zero();
}

void OptimalRotation::addVectorPair(const Vec3D &from, const Vec3D &to) {
  stack += to.normalized()^from.normalized();
  valid=false;
}

Mat3D OptimalRotation::getOptimalRotation() {
  if (!valid) {
    Mat3D L,R;
    optRot = stack;
    optRot.svd(L,R);
    double d=L.det()*R.det();
    if (d<0.0) {
      Mat3D T;
      T(2,2)=-1.0;
      optRot = L*T*R;
    } else {
      optRot = L*R;
    }
    valid = true;
  }
  return optRot;
};




Mat3D VectorPairRotation(const Vec3D& from1, const Vec3D& from2, const Vec3D& to1, const Vec3D& to2) {
  Vec3D from_x = (from1.normalized()+from2.normalized()).normalized();
  Vec3D from_y = (from1.normalized()-from2.normalized()).normalized();
  Mat3D Mfrom(from_x, from_y, from_x%from_y);

  Vec3D to_x = (to1.normalized()+to2.normalized()).normalized();
  Vec3D to_y = (to1.normalized()-to2.normalized()).normalized();
  Mat3D Mto(to_x, to_y, to_x%to_y);

  return Mto * Mfrom.inverse();
}
