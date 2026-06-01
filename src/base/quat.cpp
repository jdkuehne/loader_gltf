#include "quat.hpp"

namespace base
{

#define JK_QUAT_ALMOST_ONE 0.9999f
#define JK_QUAT_ALMOST_NEGATIVE_ONE -0.9999f

// TODO: checking and stuff?
Quat quat(F32 x, F32 y, F32 z, F32 w) {
    Quat result = {.v = {x, y, z, w}};
    return result;
}

Quat quat(F32 *v) {
    return *((Quat *)v);
}

Quat quat_ptr(F32 *p) {
    return *((Quat *)p);
}

// TODO: checking and stuff?
Quat quat_vec_scal(Vec3 v, F32 w) {
    return {.vec_scal = {v, w}};
}

Quat quat_axis_angle(Vec3 axis, F32 angle) {
    axis = vec3_normalize(axis);
    return quat_vec_scal(vec3_scale(axis, sinf(angle/2.f)), cosf(angle/2.f));
}

void quat_fill_axis_angle(Quat q, Vec3 *axis, F32 *angle) {
    if(q.w > JK_QUAT_ALMOST_ONE) {
	// NOTE(jdk): could be any axis
	*axis = vec3(1.f, 1.f, 1.f);
	*angle = 0.f;
    } else {
	F32 half_theta = acosf(q.w);
	*axis = vec3_scale(q.vec_scal.v, 1.f/sinf(half_theta));
	*angle = half_theta * 2.f;
    }
}

Quat quat_identity() {
    return quat_vec_scal(vec3_1f32(0.f), 1.f);
}

Quat quat_mul(Quat a, Quat b) {
    F32 x = (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y);
    F32 y = (a.w * b.y) + (a.y * b.w) + (a.z * b.x) - (a.x * b.z);
    F32 z = (a.w * b.z) + (a.z * b.w) + (a.x * b.y) - (a.y * b.x);
    F32 w = (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z);
    return quat(x, y, z, w);
}

Quat quat_inverse(Quat q) {
    return quat(-q.x, -q.y, -q.z, q.w);
}

Quat quat_pow(Quat q, F32 exponent) {
    F32 half_theta = acosf(q.w);
    F32 new_half_theta = half_theta * exponent;
    F32 v_correction_factor = sinf(new_half_theta) / sinf(half_theta);
    return quat_vec_scal(vec3_scale(q.vec_scal.v, v_correction_factor), cosf(new_half_theta));
}

Quat quat_diff(Quat a, Quat b) {
    return quat_mul(b, quat_inverse(a));
}

F32 quat_dot(Quat a, Quat b) {
    F32 result = 0.f;
    for(U64 i = 0; i < 4; ++i) {
	result += a.v[i]*b.v[i];
    }
    return result;
}

F32 quat_magnitude(Quat q) {
    F32 magnitude_squared = 0.f;
    for(U64 i = 0; i < 4; ++i) {
	magnitude_squared += q.v[i]*q.v[i];
    }
    return sqrtf(magnitude_squared);
}

Quat quat_scale(Quat q, F32 scalar) {
    Quat result = {};
    for(U64 i = 0; i < 4; ++i) {
	result.v[i] = q.v[i] * scalar;
    }
    return result;
}

Quat quat_normalize(Quat q) {
    F32 f = 1.f/quat_magnitude(q);
    return quat_scale(q, f);
}

static Quat _quat_add_two_scaled(Quat a, F32 ka,
				 Quat b, F32 kb) {
    Quat result = {};
    for(U64 i = 0; i < 4; ++i) {
	result.v[i] = a.v[i]*ka + b.v[i]*kb;
    }
    return result;
}
// jdk: assumes normalized quaternions
#include <stdio.h>
Quat slerp(Quat a, Quat b, F32 p) {
    // jdk: omega is the interpolation angle between the two 4-component vectors
    F32 cos_omega = quat_dot(a, b);
    if(cos_omega < JK_QUAT_ALMOST_NEGATIVE_ONE) {
	// TODO(jdk): figure out if identity if 180 deg is actually the right way to go
	return quat_identity();
    }
    if(cos_omega > JK_QUAT_ALMOST_ONE) {
	// jdk: linear interpolation if very small difference
	F32 ka = 1.0 - p;
	F32 kb = p;
	return quat_normalize(_quat_add_two_scaled(a, ka, b, kb));
    }
    F32 omega = acosf(cos_omega);
    F32 sin_omega = sqrtf(1 - cos_omega * cos_omega);
    F32 sin_omega_inverse = 1.f / sin_omega;
    F32 pa = (1 - p);
    F32 pb = p;
    F32 ka = sin(pa * omega) * sin_omega_inverse;
    F32 kb = sin(pb * omega) * sin_omega_inverse;
    return _quat_add_two_scaled(a, ka, b, kb);
}

} /*namespace base*/
