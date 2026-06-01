#ifndef MAT_H
#define MAT_H

#include "core.hpp"
#include "vec.hpp"
#include "quat.hpp"
#include "transform.hpp"

#include <math.h>
#include <stdio.h>
#include <string.h>

namespace base
{

typedef struct Mat4F32 {
    F32 v[4][4];
} Mat4F32;

typedef Mat4F32 Mat4;

// jdk: all operations treat the matrix as column-major => columns are contiguous,
// but default accessing is (ln, col) so it feels closer to the way you would do it
// in mathematics
Mat4  mat4                  (F32 diagonal);
Mat4  mat4_copy_buffer      (F32 *buf);
Mat4  mat4_f32              (F32 diagonal);
F32   mat4_get              (Mat4 *m, U64 ln, U64 col);
F32   mat4_get_xy           (Mat4 *m, U64 x, U64 y);
F32  *mat4_getp             (Mat4 *m, U64 ln, U64 col);
F32  *mat4_getp_xy          (Mat4 *m, U64 x, U64 y);
Mat4  mat4_add              (Mat4 a, Mat4 b);
Mat4  mat4_sub              (Mat4 a, Mat4 b);
Mat4  mat4_mul              (Mat4 a, Mat4 b);
Mat4  mat4_div              (Mat4 a, Mat4 b);
Mat4  mat4_inv              (Mat4 m);
Mat4  mat4_mul3             (Mat4 a, Mat4 b, Mat4 c);
Mat4  make_mat4_translate   (Vec3 t);
Mat4  make_mat4_scale       (Vec3 s);
Mat4  make_mat4_rotate      (F32 theta, Vec3 axis);
Mat4  make_mat4_rotate_quat (Quat q);
Mat4  make_mat4_perspective (F32 fov, F32 aspect_ratio, F32 near_z, F32 far_z);
Mat4  make_mat4_look_at     (Vec3 eye, Vec3 center, Vec3 up);
Mat4  make_mat4_transform   (Vec3 translation, Quat rotation, Vec3 scale);
Mat4  make_mat4_transform   (Transform t);

void mat4_print(Mat4 m);

} /*namespace base*/

#endif // MAT_H
