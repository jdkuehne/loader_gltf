#ifndef SHADER_H
#define SHADER_H

#include "jbase.hpp"
#include "camera.hpp"
#include "ext/glad/gl.h"

#define JM_BASE_TEX_INDEX_MORPH 5
#define JM_TEX_INDEX_ALBEDO 0
#define JM_TEX_INDEX_METALLIC_ROUGHNESS 1
#define JK_MAIN_VS_PATH "./src/shaders/main_vs.glsl"
#define JK_MAIN_FS_PATH "./src/shaders/main_fs.glsl"

struct OptionalSampler2DLocations {
    uint32_t has_texture;
    uint32_t sampler;
};

struct MainShader {
    uint32_t id;
    // jdk: vertex shader
    uint32_t location_projection;
    uint32_t location_view;
    uint32_t location_world;

    uint32_t location_has_skin;
    uint32_t location_joint_matrices;

    OptionalSampler2DLocations locations_morph_attrib_pos;
    OptionalSampler2DLocations locations_morph_attrib_norm;
    OptionalSampler2DLocations locations_morph_attrib_tangent;
    OptionalSampler2DLocations locations_morph_attrib_texcoord0;
    uint32_t location_morph_weights;

    // jdk: fragment shader
    uint32_t location_camera_pos;

    uint32_t location_metallic_factor;
    uint32_t location_roughness_factor;
    uint32_t location_metallic_roughness_texture;
    uint32_t location_albedo_factor;
    uint32_t location_albedo_texture;

} inline main_shader = {};

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
