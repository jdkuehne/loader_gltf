#ifndef SHADER_H
#define SHADER_H

#include "jbase.hpp"
#include "camera.hpp"
#include "ext/glad/gl.h"

#define JK_MAIN_VS_PATH "./src/shaders/main_vs.glsl"
#define JK_MAIN_FS_PATH "./src/shaders/main_fs.glsl"

struct MainShader {
    uint32_t id;
    // jdk: vertex shader
    uint32_t location_projection;
    uint32_t location_view;
    uint32_t location_world;
    uint32_t location_joint_matrices;
    uint32_t location_has_skin;
    // jdk: fragment shader
    uint32_t location_camera_position;
} inline main_shader = {};

uint32_t create_shader_vf(const char *vs_path, const char *fs_path);
uint32_t create_shader_vgf(const char *vs_path, const char *gs_path, const char *fs_path);
void delete_shader(uint32_t program);

void setup_main_shader();
void main_shader_set_view_and_camera(Camera *camera);
void main_shader_set_projection(float fov, float aspect_ratio, float near_plane, float far_plane);

#endif
