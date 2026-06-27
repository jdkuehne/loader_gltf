#ifndef JK_TRANSFORM_H
#define JK_TRANSFORM_H

#include "jmath.hpp"

struct Transform {
    Vec3 translation;
    Quat rotation;
    Vec3 scale;
};

inline Transform make_transform(Vec3 translation, Quat rotation, Vec3 scale) {
    return {translation, rotation, scale};
}

// @TODO(jdk): move to jmath
inline Mat4 make_mat4_transform(Transform t) {
    return mat4_mul3(make_mat4_translate(t.translation), make_mat4_rotate_quat(t.rotation),
	    make_mat4_scale(t.scale));
}

#endif
