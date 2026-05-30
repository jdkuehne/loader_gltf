#ifndef VEC_H
#define VEC_H

#include "core.hpp"
#include "arena.hpp"
#include <math.h>

typedef union Vec2F32 {
    struct {
	F32 x, y;
    };
    F32 v[2];
} Vec2F32;

typedef union Vec3F32 {
    struct {
	F32 x, y, z;
    };
    F32 v[3];
} Vec3F32;

typedef Vec3F32 Vec3;
typedef Vec2F32 Vec2;

// jdk: cross doesn't even exist for vec2 but for consistency it's still called cross_vec3

Vec3 vec3           (F32 x, F32 y, F32 z);
Vec3 vec3           (F32 val);
Vec3 vec3           (F32 *v);
Vec3 vec3_1f32      (F32 v);
Vec3 vec3_ptr       (F32 *p);
Vec3 vec3_rgb8      (U8 r, U8 g, U8 b);

Vec3 vec3_add       (Vec3 a, Vec3 b);
Vec3 vec3_sub       (Vec3 a, Vec3 b);
Vec3 vec3_scale     (Vec3 v, F32 scale);

F32  vec3_dot       (Vec3 a, Vec3 b);
Vec3 vec3_cross     (Vec3 a, Vec3 b);

F32  vec3_length    (Vec3 v);
Vec3 vec3_normalize (Vec3 v);

Vec3 vec3_lerp      (Vec3 a, Vec3 b, F32 f);

Vec3 vec3_zero();
Vec3 vec3_translate_default();
Vec3 vec3_scale_default();

Vec2 vec2           (F32 x, F32 y);

F32 *lerp(Arena *arena, F32 *a, F32 *b, U64 count, F32 f);

#endif // VEC_H
