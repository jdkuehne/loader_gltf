#include "vec.hpp"

namespace base
{

Vec3 vec3(F32 x, F32 y, F32 z) {
    Vec3 result = { .v = { x, y, z } };
    return result;
}

Vec3 vec3(F32 val) {
    return { .v = {val, val, val} };
}

Vec3 vec3(F32 *v) {
    return *((Vec3 *)v);
}

Vec3 vec3_translate_default() {
    return vec3(0.f, 0.f, 0.f);
}

Vec3 vec3_scale_default() {
    return vec3(1.f, 1.f, 1.f);
}

Vec3 vec3_zero() {
    return vec3(0.f, 0.f, 0.f);
}

Vec3 vec3_1f32(F32 v) {
    Vec3 result = vec3(v, v, v);
    return result;
}

Vec3 vec3_ptr(F32 *p) {
    return vec3(p[0], p[1], p[2]);
}

Vec3 vec3_rgb8(U8 r, U8 g, U8 b) {
    return vec3_scale(vec3((F32)r, (F32)g, (F32)b), 1.f/255.f);
}


Vec3 vec3_add(Vec3 a, Vec3 b) {
    Vec3 result = vec3(a.x + b.x,
		       a.y + b.y,
		       a.z + b.z);
    return result;
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    Vec3 result = vec3(a.x - b.x,
		       a.y - b.y,
		       a.z - b.z);
    return result;
}

Vec3 vec3_scale(Vec3 v, F32 scale) {
    Vec3 result = vec3(v.x * scale,
		       v.y * scale,
		       v.z * scale);
    return result;
}

F32 vec3_dot(Vec3 a, Vec3 b) {
    F32 result = (a.x * b.x +
		  a.y * b.y +
		  a.z * b.z);
    return result;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    Vec3 result = vec3(a.y * b.z - a.z * b.y,
		       a.z * b.x - a.x * b.z,
		       a.x * b.y - a.y * b.x);
    return result;
}

F32 vec3_length(Vec3 v) {
    F32 result = sqrt(v.x * v.x +
		      v.y * v.y +
		      v.z * v.z);
    return result;
}

Vec3 vec3_normalize(Vec3 v) {
    Vec3 result = vec3_scale(v, 1.f / vec3_length(v));
    return result;
}

Vec3 vec3_lerp(Vec3 a, Vec3 b, F32 f) {
    F32 f0 = 1 - f;
    F32 f1 = f;
    return vec3(
	    a.x*f0 + b.x*f1,
	    a.y*f0 + b.y*f1,
	    a.z*f0 + b.z*f1
    );

}

Vec2 vec2(F32 x, F32 y) {
    Vec2 result = {.v = {x, y}};
    return result;
}

F32 *lerp(Arena *arena, F32 *a, F32 *b, U64 count, F32 f) {
    F32 *result = arena_alloc<F32>(arena, count);
    for(U64 i = 0; i < count; ++i) {
	F32 fa = (1.f - f);
	F32 fb = f;
	result[i] = a[i]*fa + b[i]*fb;
    }
    return result;
}

} /*namespace base*/
