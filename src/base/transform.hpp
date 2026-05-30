#ifndef JK_TRANSFORM_H
#define JK_TRANSFORM_H

#include "vec.hpp"
#include "quat.hpp"

struct Transform {
    Vec3 translation;
    Quat rotation;
    Vec3 scale;
};

inline Transform make_transform(Vec3 translation, Quat rotation, Vec3 scale) {
    return {translation, rotation, scale};
}

#endif
