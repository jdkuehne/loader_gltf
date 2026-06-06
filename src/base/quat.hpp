#ifndef QUAT_H
#define QUAT_H

#include "vec.hpp"

/// DOC(jdk):
/// What i've learnt about quaternions:
/// What actually matters the most for rotation is that the quaternion is a unit vector mapping
/// of a axis angle representation, which makes the slerp possible, a interpolation
/// that takes unit vectors and then interpolates them to always be inside their unit sphere
/// => they stay unit vectors for every interpolation factor, which appears to work for any vector
///
/// TODO(jdk): write about multiplication and overall more about the complex number interpretation

typedef union QuatF32 {
    struct {
	F32 x, y, z, w;
    };
    struct {
	Vec3 v;
	F32 w;
    } vec_scal;
    F32 v[4];
} QuatF32;

// jdk: cos * x - sin * y
//	sin * x + cos * y

typedef QuatF32 Quat;

Quat quat(F32 x, F32 y, F32 z, F32 w);
Quat quat(F32 *v);
Quat quat_ptr(F32 *p);
Quat quat_vec_scal(Vec3 v, F32 w);
Quat quat_axis_angle(Vec3 axis, F32 angle);
Quat quat_identity();

void quat_fill_axis_angle(Quat q, Vec3 *axis, F32 *angle);

Quat quat_mul(Quat a, Quat b);
Quat quat_inverse(Quat q);
Quat quat_pow(Quat q, F32 exponent);
Quat quat_diff(Quat a, Quat b);
F32 quat_dot(Quat a, Quat b);
F32 quat_magnitude(Quat q);
Quat quat_scale(Quat q, F32 scalar);
Quat quat_normalize(Quat q);

// jdk: honestly the only thing that's useful here
Quat slerp(Quat a, Quat b, F32 p);

#endif
