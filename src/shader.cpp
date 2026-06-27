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

void setup_main_shader() {
    main_shader.id = create_shader_vf(JK_MAIN_VS_PATH, JK_MAIN_FS_PATH);
    main_shader.location_projection = glGetUniformLocation(main_shader.id, "proj");
    main_shader.location_view = glGetUniformLocation(main_shader.id, "view");
    main_shader.location_world = glGetUniformLocation(main_shader.id, "world");
    main_shader.location_joint_matrices = glGetUniformLocation(main_shader.id, "joint_matrices");
    main_shader.location_has_skin = glGetUniformLocation(main_shader.id, "has_skin");
    main_shader.location_camera_position = glGetUniformLocation(main_shader.id, "camera_pos");
}

void main_shader_set_view_and_camera(Camera *camera) {
    Mat4 view = camera_look_at(camera);
    glUseProgram(main_shader.id);
    glUniformMatrix4fv(main_shader.location_view, 1, GL_FALSE, (float *)&view);
    glUniform3fv(main_shader.location_camera_position, 1, (float *)&camera->pos);
    glUseProgram(0);
}

void main_shader_set_projection(float fov, float aspect_ratio, float near_plane, float far_plane) {
    Mat4 projection = make_mat4_perspective(fov, aspect_ratio, near_plane, far_plane);
    glUseProgram(main_shader.id);
    glUniformMatrix4fv(main_shader.location_projection, 1, GL_FALSE, (float *)&projection);
    glUseProgram(0);
}
