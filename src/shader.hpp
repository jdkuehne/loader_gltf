#ifndef SHADER_H
#define SHADER_H

#include "jbase.hpp"
#include "camera.hpp"
#include "ext/glad/gl.h"

#define JM_ITEX_ALBEDO 0
#define JM_ITEX_METALLIC_ROUGH 1
#define JM_ITEX_MORPH_POS 5
#define JM_ITEX_MORPH_NORM 6
#define JM_ITEX_MORPH_TANGENT 7
#define JM_ITEX_MORPH_TEXCOORD0 8

#define JK_MAIN_VS_PATH "./src/shaders/main_vs.glsl"
#define JK_MAIN_FS_PATH "./src/shaders/main_fs.glsl"

struct UBO_VS0 {
    Mat4 world;
    int has_morph_attrib_pos;
    int has_morph_attrib_norm;
    int has_morph_attrib_tangent;
    int has_morph_attrib_texcoord0;
    int has_skin;
    int padding0[3];
};

// jdk: no padding required for this one
struct UBO_FS0 {
    Vec3 albedo_factor;
    float metallic_factor;
    float roughness_factor;
    int has_albedo_texture;
    int has_metallic_channel;
    int has_roughness_channel;
};

struct MainShader {
    GLuint id;
    Mat4 joint_matrices[64];
    // jdk: vertex shader
    GLint location_projection;
    GLint location_view;

    GLint location_morph_weights;
    GLint location_joint_matrices;
    GLint location_morph_attrib_pos;
    GLint location_morph_attrib_norm;
    GLint location_morph_attrib_tangent;
    GLint location_morph_attrib_texcoord0;
    GLuint ubo_vs0;
    UBO_VS0 ubo_vs0_data;

    // jdk: fragment shader
    GLint location_camera_pos;
    GLint location_albedo_texture;
    GLint location_metallic_roughness_texture;
    GLuint ubo_fs0;
    UBO_FS0 ubo_fs0_data;
} inline main_shader = {};

/*
struct ColorMappingShader {

} inline color_mapping_shader = {};
*/

uint32_t create_shader_vf(const char *vs_path, const char *fs_path);
uint32_t create_shader_vgf(const char *vs_path, const char *gs_path, const char *fs_path);
void delete_shader(uint32_t program);

void setup_main_shader();
void main_shader_set_view_and_camera(Camera *camera);
void main_shader_set_projection(float fov, float aspect_ratio, float near_plane, float far_plane);
void main_shader_setup_texture_sampler_indices();

inline GLenum pixel_format_from_num_components(int num_components) {
    switch(num_components) {
	case 1: return GL_RED;
	case 2: return GL_RG;
	case 3: return GL_RGB;
	case 4: return GL_RGBA;
    }
    assert(0 && "weird component number");
    return 0;
}

#endif
