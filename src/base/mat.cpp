#include "mat.hpp"

Mat4 mat4(F32 diagonal) {
    Mat4 result = {0};
    for(U64 i = 0; i < 4; ++i) {
	result.v[i][i] = diagonal;
    }
    return result;
}

Mat4 mat4_copy_buffer(F32 *buf) {
    Mat4 result = {0};
    memcpy(&result, buf, sizeof(Mat4));
    return result;
}

F32  mat4_get     (Mat4 *m, U64 ln, U64 col) { return m->v[col][ln]; }
F32  mat4_get_xy  (Mat4 *m, U64 x, U64 y)    { return m->v[x][y]; }
F32 *mat4_getp    (Mat4 *m, U64 ln, U64 col) { return &m->v[col][ln]; }
F32 *mat4_getp_xy (Mat4 *m, U64 x, U64 y)    { return &m->v[x][y]; }

Mat4 mat4_mul(Mat4 a, Mat4 b) {
    Mat4 result = {0};
    for(U64 col = 0; col < 4; ++col) {
	for(U64 ln = 0; ln < 4; ++ln) {
	    *mat4_getp(&result, ln, col) =
		mat4_get(&a, ln, 0) * mat4_get(&b, 0, col) +
		mat4_get(&a, ln, 1) * mat4_get(&b, 1, col) +
		mat4_get(&a, ln, 2) * mat4_get(&b, 2, col) +
		mat4_get(&a, ln, 3) * mat4_get(&b, 3, col);
	}
    }
    return result;
}

// Mat4 mat4_div(Mat4 m) {
//
// }

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

Mat4 make_mat4_rotate(F32 theta, Vec3 axis) {
    Mat4 result = mat4(1.f);
    axis = vec3_normalize(axis);
    F32 cos_theta = cosf(theta);
    F32 sin_theta = sinf(theta);

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
    F32 angle = 0;
    Vec3 axis = {0};
    quat_fill_axis_angle(q, &axis, &angle);
    return make_mat4_rotate(angle, axis);
}

// TODO: quaternion function that creates rotation matrix
// Mat4 make_mat4_quat(

Mat4 make_mat4_perspective(F32 fov, F32 aspect_ratio, F32 near_z, F32 far_z) {
    Mat4 result = mat4(0.f);
    F32 tan_theta_over_2 = tanf(fov / 2);
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

Mat4 make_mat4_transform(Transform t) {
    return mat4_mul3(make_mat4_translate(t.translation), make_mat4_rotate_quat(t.rotation),
	    make_mat4_scale(t.scale));
}

void mat4_print(Mat4 m) {
    for(U64 y = 0; y < 4; ++y) {
	for(U64 x = 0; x < 4; ++x) {
	    printf("%.3f,", mat4_get(&m, y, x));
	}
	putchar('\n');
    }
}
/*
matrix2x2 multiplication:
result = {
    m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0],   m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1],

    m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0],   m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1],
};
*/
