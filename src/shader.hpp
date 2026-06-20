#ifndef SHADER_H
#define SHADER_H

#include "base/core.hpp"
#include "base/os/file.hpp"
#include "base/mem/allocator.hpp"
#include "camera.hpp"
#include "ext/glad/gl.h"

#define JK_MAIN_VS_PATH "./src/shaders/main_vs.glsl"
#define JK_MAIN_FS_PATH "./src/shaders/main_fs.glsl"

struct {
    U32 id;
    // jdk: vertex shader
    U32 location_projection;
    U32 location_view;
    U32 location_world;
    U32 location_joint_matrices;
    U32 location_has_skin;

    // jdk: fragment shader
    U32 location_camera_position;
} inline main_shader = {};

U32 create_shader_vf(const char *vs_path, const char *fs_path);
U32 create_shader_vgf(const char *vs_path, const char *gs_path, const char *fs_path);
void delete_shader(U32 program);

void setup_main_shader();
void main_shader_set_view_and_camera(Camera *camera);
void main_shader_set_projection(F32 fov, F32 aspect_ratio, F32 near_plane, F32 far_plane);

#endif
