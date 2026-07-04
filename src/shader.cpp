#include "shader.hpp"

#include <stdio.h>
#include <stdlib.h>

// @TODO(jdk): allocators are implicit defaults, change that?

static int compile_shader_file(GLenum type, const char *path, uint32_t *shader_out) {
    Str8 source = file_read_full_to_str8(str8(path));
    int compilation_success;
    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&source.ptr, (const int *)&source.len);
    release(source.ptr);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_success);
    if(!compilation_success) {
	char infoLog[512];
	glGetShaderInfoLog(shader, 512, NULL, infoLog);
	glDeleteShader(shader);
	puts(infoLog);
	return JM_ERROR_GENERAL;
    }
    *shader_out = shader;
    return JM_SUCCESS;
}

uint32_t create_shader_vf(const char *vs_path, const char *fs_path) {
    //shader compilation
    int compile_err;
    uint32_t vs, fs;
    compile_err = compile_shader_file(GL_VERTEX_SHADER, vs_path, &vs);
    if(compile_err) exit(JM_ERROR_SHADERCOMPILE_VS);

    compile_err = compile_shader_file(GL_FRAGMENT_SHADER, fs_path, &fs);
    if(compile_err) exit(JM_ERROR_SHADERCOMPILE_FS);

    //shader linking
    int link_success;
    uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glGetProgramiv(program, GL_LINK_STATUS, &link_success);
    if(!link_success) {
	char infoLog[512];
	glGetProgramInfoLog(program, 512, NULL, infoLog);
	glDeleteProgram(program);
	puts(infoLog);
	exit(JM_ERROR_SHADERPROGRAMLINK);
    }
    return program;
}

uint32_t create_shader_vgf(const char *vs_path, const char *gs_path, const char *fs_path) {
    //shader compilation
    int compile_err;
    uint32_t vs, fs, gs;
    compile_err = compile_shader_file(GL_VERTEX_SHADER, vs_path, &vs);
    if(compile_err) exit(JM_ERROR_SHADERCOMPILE_VS);

    compile_err = compile_shader_file(GL_GEOMETRY_SHADER, gs_path, &gs);
    if(compile_err) exit(JM_ERROR_SHADERCOMPILE_GS);

    compile_err = compile_shader_file(GL_FRAGMENT_SHADER, fs_path, &fs);
    if(compile_err) exit(JM_ERROR_SHADERCOMPILE_FS);

    //shader linking
    int link_success;
    uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, gs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(gs);
    glDeleteShader(fs);
    glGetProgramiv(program, GL_LINK_STATUS, &link_success);
    if(!link_success) {
	char infoLog[512];
	glGetProgramInfoLog(program, 512, NULL, infoLog);
	glDeleteProgram(program);
	printf("%s\n", infoLog);
	exit(JM_ERROR_SHADERPROGRAMLINK); 
    }
    return program;
}

void delete_shader(uint32_t program) {
    glDeleteProgram(program);
}

static GLint get_loc(GLuint id, const char *name) {
    GLint result = glGetUniformLocation(id, name);
    if(result == -1) {
	printf("could not find uniform: %s\n", name);
    }
    return result;
}

static OptionalSampler2DLocations get_opt_sampler2d_locations(uint32_t program,
	const char *struct_name) {
    return {
	get_loc(program, cstr_cat(struct_name, ".has_texture", &temp_allocator)),
 	get_loc(program, cstr_cat(struct_name, ".sampler", &temp_allocator))
    };
}
void setup_main_shader() {
    MainShader &ms = main_shader;
    ms.id = create_shader_vf(JK_MAIN_VS_PATH, JK_MAIN_FS_PATH);
    ms.location_projection             = get_loc(ms.id, "projection");
    ms.location_view                   = get_loc(ms.id, "view");
    ms.location_world                  = get_loc(ms.id, "world");
    ms.location_has_skin               = get_loc(ms.id, "has_skin");
    ms.location_joint_matrices         = get_loc(ms.id, "joint_matrices");

    ms.locations_morph_attrib_pos       = get_opt_sampler2d_locations(ms.id, "morph_attrib_pos");
    ms.locations_morph_attrib_norm      = get_opt_sampler2d_locations(ms.id, "morph_attrib_norm");
    ms.locations_morph_attrib_tangent   = get_opt_sampler2d_locations(ms.id, "morph_attrib_tangent");
    ms.locations_morph_attrib_texcoord0 = get_opt_sampler2d_locations(ms.id, "morph_attrib_texcoord0");
    ms.location_morph_weights          = get_loc(ms.id, "morph_weights");

    ms.location_camera_pos             = get_loc(ms.id, "camera_pos");
    ms.location_metallic_factor        = get_loc(ms.id, "metallic_factor");
    ms.location_roughness_factor       = get_loc(ms.id, "roughness_factor");
    ms.locations_metallic_roughness_texture = get_opt_sampler2d_locations(ms.id, "metallic_roughness_texture");
    ms.location_albedo_factor          = get_loc(ms.id, "albedo_factor");
    ms.locations_albedo_texture         = get_opt_sampler2d_locations(ms.id, "albedo_texture");
}

// jdk: those don't change and are important so a picture doesn't get used in a morph target or
// something
void main_shader_setup_texture_sampler_indices() {
    glUseProgram(main_shader.id);
    glUniform1i(main_shader.locations_albedo_texture.sampler,             JM_ITEX_ALBEDO);
    glUniform1i(main_shader.locations_metallic_roughness_texture.sampler, JM_ITEX_METALLIC_ROUGH);
    glUniform1i(main_shader.locations_morph_attrib_pos.sampler,           JM_ITEX_MORPH_POS);
    glUniform1i(main_shader.locations_morph_attrib_norm.sampler,          JM_ITEX_MORPH_NORM);
    glUniform1i(main_shader.locations_morph_attrib_tangent.sampler,       JM_ITEX_MORPH_TANGENT);
    glUniform1i(main_shader.locations_morph_attrib_texcoord0.sampler,     JM_ITEX_MORPH_TEXCOORD0);
    glUseProgram(0);
}

void main_shader_set_view_and_camera(Camera *camera) {
    Mat4 view = camera_look_at(camera);
    glUseProgram(main_shader.id);
    glUniformMatrix4fv(main_shader.location_view, 1, GL_FALSE, (float *)&view);
    glUniform3fv(main_shader.location_camera_pos, 1, (float *)&camera->pos);
    glUseProgram(0);
}

void main_shader_set_projection(float fov, float aspect_ratio, float near_plane, float far_plane) {
    Mat4 projection = make_mat4_perspective(fov, aspect_ratio, near_plane, far_plane);
    glUseProgram(main_shader.id);
    glUniformMatrix4fv(main_shader.location_projection, 1, GL_FALSE, (float *)&projection);
    glUseProgram(0);
}
