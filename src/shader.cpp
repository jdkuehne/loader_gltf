#include "shader.hpp"

#include "base/mat.hpp"
#include "base/mem/allocator.hpp"

#include <stdio.h>
#include <stdlib.h>

// @TODO(jdk): allocators are implicit defaults, change that?

static I32 compile_shader_file(GLenum type, const char *path, U32 *shader_out) {
    Str8 source = file_read_full_to_str8(str8c(path));
    I32 compilation_success;
    U32 shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&source.start, (const I32 *)&source.len);
    mem_free(source.start);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_success);
    if(!compilation_success) {
	char infoLog[512];
	glGetShaderInfoLog(shader, 512, NULL, infoLog);
	glDeleteShader(shader);
	puts(infoLog);
	return JK_ERROR_GENERAL;
    }
    *shader_out = shader;
    return JK_SUCCESS;
}

U32 create_shader_vf(const char *vs_path, const char *fs_path) {
    //shader compilation
    I32 compile_err;
    U32 vs, fs;
    compile_err = compile_shader_file(GL_VERTEX_SHADER, vs_path, &vs);
    if(compile_err) exit(JK_ERROR_SHADERCOMPILE_VS);

    compile_err = compile_shader_file(GL_FRAGMENT_SHADER, fs_path, &fs);
    if(compile_err) exit(JK_ERROR_SHADERCOMPILE_FS);

    //shader linking
    I32 link_success;
    U32 program = glCreateProgram();
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
	exit(JK_ERROR_SHADERPROGRAMLINK);
    }
    return program;
}

U32 create_shader_vgf(const char *vs_path, const char *gs_path, const char *fs_path) {
    //shader compilation
    I32 compile_err;
    U32 vs, fs, gs;
    compile_err = compile_shader_file(GL_VERTEX_SHADER, vs_path, &vs);
    if(compile_err) exit(JK_ERROR_SHADERCOMPILE_VS);

    compile_err = compile_shader_file(GL_GEOMETRY_SHADER, gs_path, &gs);
    if(compile_err) exit(JK_ERROR_SHADERCOMPILE_GS);

    compile_err = compile_shader_file(GL_FRAGMENT_SHADER, fs_path, &fs);
    if(compile_err) exit(JK_ERROR_SHADERCOMPILE_FS);
    
    //shader linking
    I32 link_success;
    U32 program = glCreateProgram();
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
	exit(JK_ERROR_SHADERPROGRAMLINK); 
    }
    return program;
}

void delete_shader(U32 program) {
    glDeleteProgram(program);
}

void setup_main_shader() {
    main_shader.id = create_shader_vf(JK_MAIN_VS_PATH, JK_MAIN_FS_PATH);
    main_shader.location_projection = glGetUniformLocation(main_shader.id, "proj");
    main_shader.location_view = glGetUniformLocation(main_shader.id, "view");
    main_shader.location_world = glGetUniformLocation(main_shader.id, "world");
    main_shader.location_joint_matrices = glGetUniformLocation(main_shader.id, "joint_matrices");
    main_shader.location_has_skin = glGetUniformLocation(main_shader.id, "has_skin");
}

void main_shader_set_view_and_camera(Camera *camera) {
    Mat4 view = camera_look_at(camera);
    glUseProgram(main_shader.id);
    glUniformMatrix4fv(main_shader.location_view, 1, GL_FALSE, (F32 *)&view);
    glUniform3fv(main_shader.location_camera_position, 1, (F32 *)&camera->pos);
    glUseProgram(0);
}

void main_shader_set_projection(F32 fov, F32 aspect_ratio, F32 near_plane, F32 far_plane) {
    Mat4 projection = make_mat4_perspective(fov, aspect_ratio, near_plane, far_plane);
    glUseProgram(main_shader.id);
    glUniformMatrix4fv(main_shader.location_projection, 1, GL_FALSE, (F32 *)&projection);
    glUseProgram(0);
}
