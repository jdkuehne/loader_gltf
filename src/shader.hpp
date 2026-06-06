#ifndef SHADER_H
#define SHADER_H

#include "base/core.hpp"
#include "base/os/file.hpp"
#include "base/mem/allocator.hpp"

#include "ext/glad/gl.h"


U32 create_shader_vf(const char *vs_path, const char *fs_path);
U32 create_shader_vgf(const char *vs_path, const char *gs_path, const char *fs_path);
void delete_shader(U32 program);

/*

////////////////////////////////////////
// simple type uniforms
void uniform_i32(U32 program, const char *uniform_name, I32 value);
void uniform_f32(U32 program, const char *uniform_name, F32 value);
void uniform_f32_num3(U32 program, const char *uniform_name, F32 first, F32 second, F32 third);
void uniform_f32_vec3(U32 program, const char *uniform_name, const F32 *values);
void uniform_f32_mat4x4(U32 program, const char *uniform_name, const F32 *values);

////////////////////////////////////////
// glm types
void uniform_glm_vec3(U32 program, const char *uniform_name, const glm::vec3 *vector);
void uniform_glm_mat4(U32 program, const char *uniform_name, const glm::mat4 *matrix);

*/

#endif



