#ifndef JK_CAMERA_H
#define JK_CAMERA_H

#include "jmath.hpp"

const inline Vec3 camera_up = vec3(0.0, 1.0, 0.0);

struct Camera {
    Vec3 pos;
    Vec3 dir;
    float pitch;
    float yaw;
};

inline void _camera_update_direction(Camera *camera) {
    float x = sinf(camera->yaw) * cosf(camera->pitch);
    float y = sinf(camera->pitch);
    float z = -cosf(camera->yaw) * cosf(camera->pitch);
    camera->dir = vec3_normalize(vec3(x, y, z));
}

inline Camera make_camera(Vec3 pos = vec3(0.f), float pitch = 0.f, float yaw = 0.f) {
    Camera result = {pos, vec3(0.f), pitch, yaw};
    _camera_update_direction(&result);
    return result;
}

inline void camera_set_pitch(Camera *camera, float pitch) {
    camera->pitch = pitch;
    _camera_update_direction(camera);
}

inline void camera_add_pitch(Camera *camera, float pitch) {
    camera->pitch += pitch;
    _camera_update_direction(camera);
}

inline void camera_set_yaw(Camera *camera, float yaw) {
    camera->yaw = yaw;
    _camera_update_direction(camera);
}

inline void camera_add_yaw(Camera *camera, float yaw) {
    camera->yaw += yaw;
    _camera_update_direction(camera);
}

inline void camera_move(Camera *camera, Vec3 move) {
    camera->pos = vec3_add(camera->pos, move);
}

// jdk: move like in first person shooter => left = cross(dir, up)...
inline void camera_fps_move(Camera *camera, Vec3 move) {
    // jdk: apply side motion
    camera->pos = vec3_add(camera->pos, vec3_scale(vec3_cross(camera->dir, camera_up), move.x));
    // jdk: apply up/down motion
    camera->pos = vec3_add(camera->pos, vec3_scale(camera_up, move.y));
    // jdk: apply front motion
    camera->pos = vec3_add(camera->pos, vec3_scale(camera->dir, move.z));
}

inline void camera_fps_turn(Camera *camera, Vec2 r) {
    camera->yaw += r.x;
    camera->pitch += r.y;
    _camera_update_direction(camera);
}

inline Mat4 camera_look_at(Camera *camera) {
    return make_mat4_look_at(camera->pos, vec3_add(camera->pos, camera->dir), camera_up);
}

#endif
