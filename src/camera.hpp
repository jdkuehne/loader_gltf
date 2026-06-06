#ifndef JK_CAMERA_H
#define JK_CAMERA_H

#include "base/vec.hpp"
#include "base/mat.hpp"

const inline Vec3 camera_up = vec3(0.0, 1.0, 0.0);

struct Camera {
    Vec3 pos;
    Vec3 dir;
    F32 pitch;
    F32 yaw;
};

inline void _camera_update_direction(Camera *camera) {
    F32 x = sinf(camera->yaw) * cosf(camera->pitch);
    F32 y = sinf(camera->pitch);
    F32 z = -cosf(camera->yaw) * cosf(camera->pitch);
    camera->dir = vec3_normalize(vec3(x, y, z));
}

inline Camera make_camera(Vec3 pos, F32 pitch, F32 yaw) {
    Camera result = {pos, vec3(0.f), pitch, yaw};
    _camera_update_direction(&result);
    return result;
}


inline void camera_set_pitch(Camera *camera, F32 pitch) {
    camera->pitch = pitch;
    _camera_update_direction(camera);
}

inline void camera_add_pitch(Camera *camera, F32 pitch) {
    camera->pitch += pitch;
    _camera_update_direction(camera);
}

inline void camera_set_yaw(Camera *camera, F32 yaw) {
    camera->yaw = yaw;
    _camera_update_direction(camera);
}

inline void camera_add_yaw(Camera *camera, F32 yaw) {
    camera->yaw += yaw;
    _camera_update_direction(camera);
}

inline void camera_move(Camera *camera, Vec3 move) {
    camera->pos = vec3_add(camera->pos, move);
}

inline Mat4 camera_look_at(Camera *camera) {
    return make_mat4_look_at(camera->pos, vec3_add(camera->pos, camera->dir), camera_up);
}

#endif
