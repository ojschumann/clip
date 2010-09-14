#include "optimalrotation.h"

OptimalRotation::OptimalRotation()
{
    reset();
}

void OptimalRotation::reset() {
    valid = false;
    stack.zero();
}

void OptimalRotation::addVectorPair(const Vec3D &v1, const Vec3D &v2) {
    stack += v1.normalized()^v2.normalized();
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
            T.at(2,2)=-1.0;
            optRot = L*T*R;
        } else {
            optRot = L*R;
        }
        optRot = optRot.orthogonalize();
        valid = true;
    }
    return optRot;
};
