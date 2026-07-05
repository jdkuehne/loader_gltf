#ifndef JMATH_HPP
#define JMATH_HPP

#include "jbase.hpp"

// @TAG(jdk): vec2 decl
#define JK_CompPasteVec3(v) (v).x, (v).y, (v).z
#define JK_CompPasteVec2(v) (v).x, (v).y

typedef union Vec2F32 {
    struct {
	float x, y;
    };
    float v[2];
} Vec2F32;

typedef union Vec3F32 {
    struct {
	float x, y, z;
    };
    float v[3];
} Vec3F32;

typedef Vec3F32 Vec3;
typedef Vec2F32 Vec2;

static_assert(sizeof(Vec2) == 8);
static_assert(sizeof(Vec3) == 12);

// jdk: cross doesn't even exist for vec2 but for consistency it's still called cross_vec3

Vec3 vec3           (float x, float y, float z);
Vec3 vec3           (float val);
Vec3 vec3           (float *v);
Vec3 vec3_ptr       (float *p);
Vec3 vec3_rgb8      (uint8_t r, uint8_t g, uint8_t b);
Vec3 vec3_add       (Vec3 a, Vec3 b);
Vec3 vec3_sub       (Vec3 a, Vec3 b);
Vec3 vec3_scale     (Vec3 v, float scale);
float  vec3_dot       (Vec3 a, Vec3 b);
Vec3 vec3_cross     (Vec3 a, Vec3 b);
float  vec3_length    (Vec3 v);
Vec3 vec3_normalize (Vec3 v);
Vec3 vec3_lerp      (Vec3 a, Vec3 b, float f);
Vec3 vec3_zero();
Vec3 vec3_translate_default();
Vec3 vec3_scale_default();

Vec2 vec2           (float x, float y);
Vec2 vec2           (float val);
Vec2 vec2_scale     (Vec2 v, float k);
Vec2 vec2_normalize (Vec2 v);

float *lerp(float *a, float *b, uint64_t count, float f, Allocator *allocator);

// @TAG(jdk): quat decl
typedef union QuatF32 {
    struct {
	float x, y, z, w;
    };
    struct {
	Vec3 v;
	float w;
    } vec_scal;
    float v[4];
} QuatF32;
typedef QuatF32 Quat;

Quat quat(float x, float y, float z, float w);
Quat quat(float *v);
Quat quat_ptr(float *p);
Quat quat_vec_scal(Vec3 v, float w);
Quat quat_axis_angle(Vec3 axis, float angle);
Quat quat_identity();
void quat_fill_axis_angle(Quat q, Vec3 *axis, float *angle);
Quat quat_mul(Quat a, Quat b);
Quat quat_inverse(Quat q);
Quat quat_pow(Quat q, float exponent);
Quat quat_diff(Quat a, Quat b);
float quat_dot(Quat a, Quat b);
float quat_magnitude(Quat q);
Quat quat_scale(Quat q, float scalar);
Quat quat_normalize(Quat q);
Quat slerp(Quat a, Quat b, float p);

// @TAG(jdk): mat4 decl
typedef struct Mat4F32 {
    float v[4][4];
} Mat4F32;

typedef Mat4F32 Mat4;

// jdk: all operations treat the matrix as column-major => columns are contiguous,
// but default accessing is (ln, col) so it feels closer to the way you would do it
// in mathematics
Mat4  mat4                  (float diagonal);
Mat4  mat4_copy_buffer      (float *buf);
Mat4  mat4_f32              (float diagonal);
float   mat4_get              (Mat4 *m, uint64_t ln, uint64_t col);
float   mat4_get_xy           (Mat4 *m, uint64_t x, uint64_t y);
float  *mat4_getp             (Mat4 *m, uint64_t ln, uint64_t col);
float  *mat4_getp_xy          (Mat4 *m, uint64_t x, uint64_t y);
Mat4  mat4_add              (Mat4 a, Mat4 b);
Mat4  mat4_sub              (Mat4 a, Mat4 b);
Mat4 mat4_mul(const Mat4 &a, const Mat4 &b);
Mat4  mat4_div              (Mat4 a, Mat4 b);
Mat4  mat4_inv              (Mat4 m);
Mat4  mat4_mul3             (Mat4 a, Mat4 b, Mat4 c);
Mat4  make_mat4_translate   (Vec3 t);
Mat4  make_mat4_scale       (Vec3 s);
Mat4  make_mat4_rotate      (float theta, Vec3 axis);
Mat4  make_mat4_rotate_quat (Quat q);
Mat4  make_mat4_perspective (float fov, float aspect_ratio, float near_z, float far_z);
Mat4  make_mat4_look_at     (Vec3 eye, Vec3 center, Vec3 up);
Mat4  make_mat4_transform   (Vec3 translation, Quat rotation, Vec3 scale);
void mat4_print(Mat4 m);

#endif

#ifdef JMATH_IMPLEMENTATION

// @TAG(jdk): vec2 impl
Vec2 vec2(float x, float y) {
    Vec2 result = {.v = {x, y}};
    return result;
}

Vec2 vec2(float val) {
    return vec2(val, val);
}

Vec2 vec2_scale(Vec2 v, float k) {
    return vec2(v.x*k, v.y*k);
}

Vec2 vec2_normalize(Vec2 v) {
    float length = sqrtf(jm_sq(v.x) + jm_sq(v.y));
    if(length < 0.001f) {
	return vec2(0.f);
    }
    return vec2_scale(v, 1.f/length);
}

// @TAG(jdk): vec3 impl
Vec3 vec3(float x, float y, float z) {
    Vec3 result = { .v = { x, y, z } };
    return result;
}

Vec3 vec3(float val) {
    return { .v = {val, val, val} };
}

Vec3 vec3(float *v) {
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

Vec3 vec3_ptr(float *p) {
    return vec3(p[0], p[1], p[2]);
}

Vec3 vec3_rgb8(uint8_t r, uint8_t g, uint8_t b) {
    return vec3_scale(vec3((float)r, (float)g, (float)b), 1.f/255.f);
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

Vec3 vec3_scale(Vec3 v, float scale) {
    Vec3 result = vec3(v.x * scale,
		       v.y * scale,
		       v.z * scale);
    return result;
}

float vec3_dot(Vec3 a, Vec3 b) {
    float result = (a.x * b.x +
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

float vec3_length(Vec3 v) {
    return sqrtf(jm_sq(v.x) + jm_sq(v.y) + jm_sq(v.z));
}

Vec3 vec3_normalize(Vec3 v) {
    float length = vec3_length(v);
    if(length < 0.001f) {
	return vec3(0.f);
    }
    return vec3_scale(v, 1.f/length);
}

Vec3 vec3_lerp(Vec3 a, Vec3 b, float f) {
    float f0 = 1 - f;
    float f1 = f;
    return vec3(
	    a.x*f0 + b.x*f1,
	    a.y*f0 + b.y*f1,
	    a.z*f0 + b.z*f1
    );

}

// @TAG(jdk): random size vec impl
float *lerp(float *a, float *b, uint64_t count, float f, Allocator *allocator) {
    float *result = alloc<float>(count, allocator);
    for(uint64_t i = 0; i < count; ++i) {
	float fa = (1.f - f);
	float fb = f;
	result[i] = a[i]*fa + b[i]*fb;
    }
    return result;
}

// @TAG(jdk): quat impl
#define JK_QUAT_ALMOST_ONE 0.9999f
#define JK_QUAT_ALMOST_NEGATIVE_ONE -0.9999f

// TODO: checking and stuff?
Quat quat(float x, float y, float z, float w) {
    Quat result = {.v = {x, y, z, w}};
    return result;
}

Quat quat(float *v) {
    return *((Quat *)v);
}

Quat quat_ptr(float *p) {
    return *((Quat *)p);
}

// TODO: checking and stuff?
Quat quat_vec_scal(Vec3 v, float w) {
    return {.vec_scal = {v, w}};
}

Quat quat_axis_angle(Vec3 axis, float angle) {
    axis = vec3_normalize(axis);
    return quat_vec_scal(vec3_scale(axis, sinf(angle/2.f)), cosf(angle/2.f));
}

void quat_fill_axis_angle(Quat q, Vec3 *axis, float *angle) {
    if(q.w > JK_QUAT_ALMOST_ONE) {
	// NOTE(jdk): could be any axis
	*axis = vec3(1.f, 1.f, 1.f);
	*angle = 0.f;
    } else {
	float half_theta = acosf(q.w);
	*axis = vec3_scale(q.vec_scal.v, 1.f/sinf(half_theta));
	*angle = half_theta * 2.f;
    }
}

Quat quat_identity() {
    return quat_vec_scal(vec3(0.f), 1.f);
}

Quat quat_mul(Quat a, Quat b) {
    float x = (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y);
    float y = (a.w * b.y) + (a.y * b.w) + (a.z * b.x) - (a.x * b.z);
    float z = (a.w * b.z) + (a.z * b.w) + (a.x * b.y) - (a.y * b.x);
    float w = (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z);
    return quat(x, y, z, w);
}

Quat quat_inverse(Quat q) {
    return quat(-q.x, -q.y, -q.z, q.w);
}

Quat quat_pow(Quat q, float exponent) {
    float half_theta = acosf(q.w);
    float new_half_theta = half_theta * exponent;
    float v_correction_factor = sinf(new_half_theta) / sinf(half_theta);
    return quat_vec_scal(vec3_scale(q.vec_scal.v, v_correction_factor), cosf(new_half_theta));
}

Quat quat_diff(Quat a, Quat b) {
    return quat_mul(b, quat_inverse(a));
}

float quat_dot(Quat a, Quat b) {
    float result = 0.f;
    for(uint64_t i = 0; i < 4; ++i) {
	result += a.v[i]*b.v[i];
    }
    return result;
}

float quat_magnitude(Quat q) {
    float magnitude_squared = 0.f;
    for(uint64_t i = 0; i < 4; ++i) {
	magnitude_squared += q.v[i]*q.v[i];
    }
    return sqrtf(magnitude_squared);
}

Quat quat_scale(Quat q, float scalar) {
    Quat result = {};
    for(uint64_t i = 0; i < 4; ++i) {
	result.v[i] = q.v[i] * scalar;
    }
    return result;
}

Quat quat_normalize(Quat q) {
    float f = 1.f/quat_magnitude(q);
    return quat_scale(q, f);
}

static Quat _quat_add_two_scaled(Quat a, float ka,
				 Quat b, float kb) {
    Quat result = {};
    for(uint64_t i = 0; i < 4; ++i) {
	result.v[i] = a.v[i]*ka + b.v[i]*kb;
    }
    return result;
}
// jdk: assumes normalized quaternions
Quat slerp(Quat a, Quat b, float p) {
    // jdk: omega is the interpolation angle between the two 4-component vectors
    float cos_omega = quat_dot(a, b);
    if(cos_omega < JK_QUAT_ALMOST_NEGATIVE_ONE) {
	// TODO(jdk): figure out if identity if 180 deg is actually the right way to go
	return quat_identity();
    }
    if(cos_omega > JK_QUAT_ALMOST_ONE) {
	// jdk: linear interpolation if very small difference
	float ka = 1.f - p;
	float kb = p;
	return quat_normalize(_quat_add_two_scaled(a, ka, b, kb));
    }
    float omega = acosf(cos_omega);
    float sin_omega = sqrtf(1 - cos_omega * cos_omega);
    float sin_omega_inverse = 1.f / sin_omega;
    float pa = (1 - p);
    float pb = p;
    float ka = sinf(pa * omega) * sin_omega_inverse;
    float kb = sinf(pb * omega) * sin_omega_inverse;
    return _quat_add_two_scaled(a, ka, b, kb);
}

// @TAG(jdk): mat4 impl
Mat4 mat4(float diagonal) {
    Mat4 result = {0};
    for(uint64_t i = 0; i < 4; ++i) {
	result.v[i][i] = diagonal;
    }
    return result;
}

Mat4 mat4_copy_buffer(float *buf) {
    Mat4 result = {0};
    memcpy(&result, buf, sizeof(Mat4));
    return result;
}

float  mat4_get     (Mat4 *m, uint64_t ln, uint64_t col) { return m->v[col][ln]; }
float  mat4_get_xy  (Mat4 *m, uint64_t x, uint64_t y)    { return m->v[x][y]; }
float *mat4_getp    (Mat4 *m, uint64_t ln, uint64_t col) { return &m->v[col][ln]; }
float *mat4_getp_xy (Mat4 *m, uint64_t x, uint64_t y)    { return &m->v[x][y]; }

Mat4 mat4_mul(const Mat4 &a, const Mat4 &b) {
    Mat4 result = {0};
    for(uint64_t col = 0; col < 4; ++col) {
	for(uint64_t ln = 0; ln < 4; ++ln) {
	    result.v[col][ln] =
		a.v[0][ln] * b.v[col][0] +
		a.v[1][ln] * b.v[col][1] +
		a.v[2][ln] * b.v[col][2] +
		a.v[3][ln] * b.v[col][3];
	}
    }
    return result;
}

Mat4 mat4_mul3(Mat4 a, Mat4 b, Mat4 c) {
    Mat4 result = mat4_mul(a, mat4_mul(b, c));
    return result;
}

Mat4 make_mat4_translate(Vec3 t) {
    Mat4 result = mat4(1.f);
    *mat4_getp(&result, 0, 3) = t.x;
    *mat4_getp(&result, 1, 3) = t.y;
    *mat4_getp(&result, 2, 3) = t.z;
    return result;
}

Mat4 make_mat4_scale(Vec3 s) {
    Mat4 result = mat4(1.f);
    *mat4_getp(&result, 0, 0) = s.x;
    *mat4_getp(&result, 1, 1) = s.y;
    *mat4_getp(&result, 2, 2) = s.z;
    return result;
}

Mat4 make_mat4_rotate(float theta, Vec3 axis) {
    Mat4 result = mat4(1.f);
    axis = vec3_normalize(axis);
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);

    *mat4_getp(&result, 0, 0) = axis.x * axis.x + (1 - axis.x * axis.x) * cos_theta + 0;
    *mat4_getp(&result, 0, 1) = axis.x * axis.y - axis.x * axis.y       * cos_theta - axis.z * sin_theta;
    *mat4_getp(&result, 0, 2) = axis.x * axis.z - axis.x * axis.z       * cos_theta + axis.y * sin_theta;

    *mat4_getp(&result, 1, 0) = axis.y * axis.x - axis.y * axis.x       * cos_theta + axis.z * sin_theta;
    *mat4_getp(&result, 1, 1) = axis.y * axis.y + (1 - axis.y * axis.y) * cos_theta + 0;
    *mat4_getp(&result, 1, 2) = axis.y * axis.z - axis.y * axis.z       * cos_theta - axis.x * sin_theta;

    *mat4_getp(&result, 2, 0) = axis.z * axis.x - axis.z * axis.x       * cos_theta - axis.y * sin_theta;
    *mat4_getp(&result, 2, 1) = axis.z * axis.y - axis.z * axis.y       * cos_theta + axis.x * sin_theta;
    *mat4_getp(&result, 2, 2) = axis.z * axis.z + (1 - axis.z * axis.z) * cos_theta + 0;
    return result;
}

// TODO(jdk): find a way to do this without angle axis?
Mat4 make_mat4_rotate_quat(Quat q) {
    float angle = 0;
    Vec3 axis = {};
    quat_fill_axis_angle(q, &axis, &angle);
    return make_mat4_rotate(angle, axis);
}

// @TODO(jdk): quaternion function that creates rotation matrix
// Mat4 make_mat4_quat(

Mat4 make_mat4_perspective(float fov, float aspect_ratio, float near_z, float far_z) {
    Mat4 result = mat4(0.f);
    float tan_theta_over_2 = tanf(fov / 2);
    *mat4_getp(&result, 0, 0) = 1.f / (aspect_ratio * tan_theta_over_2);
    *mat4_getp(&result, 1, 1) = 1.f / tan_theta_over_2;
    *mat4_getp(&result, 2, 2) = - (far_z + near_z) / (far_z - near_z);
    *mat4_getp(&result, 2, 3) = - (2.f * far_z * near_z) / (far_z - near_z);
    *mat4_getp(&result, 3, 2) = -1.f;
    return result;
}

Mat4 make_mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
    Mat4 result = mat4(1.f);
    Vec3 cam_back  = vec3_normalize(vec3_sub(eye, center));
    Vec3 cam_right = vec3_normalize(vec3_cross(up, cam_back));
    Vec3 cam_up    = vec3_cross(cam_back, cam_right);

    *mat4_getp(&result, 0, 0) = cam_right.x;
    *mat4_getp(&result, 0, 1) = cam_right.y;
    *mat4_getp(&result, 0, 2) = cam_right.z;

    *mat4_getp(&result, 1, 0) = cam_up.x;
    *mat4_getp(&result, 1, 1) = cam_up.y;
    *mat4_getp(&result, 1, 2) = cam_up.z;

    *mat4_getp(&result, 2, 0) = cam_back.x;
    *mat4_getp(&result, 2, 1) = cam_back.y;
    *mat4_getp(&result, 2, 2) = cam_back.z;

    *mat4_getp(&result, 0, 3) = -vec3_dot(eye, cam_right);
    *mat4_getp(&result, 1, 3) = -vec3_dot(eye, cam_up);
    *mat4_getp(&result, 2, 3) = -vec3_dot(eye, cam_back);
    return result;
}

Mat4 make_mat4_transform(Vec3 translation, Quat rotation, Vec3 scale) {
    return mat4_mul3(make_mat4_translate(translation), make_mat4_rotate_quat(rotation),
	    make_mat4_scale(scale));
}

void mat4_print(Mat4 m) {
    for(uint64_t y = 0; y < 4; ++y) {
	for(uint64_t x = 0; x < 4; ++x) {
	    printf("%.3f,", mat4_get(&m, y, x));
	}
	putchar('\n');
    }
}

#endif
