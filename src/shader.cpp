#include "shader.hpp"

static I64 fsize(I32 fd)
{
    struct stat filestatus;
    I32 err = fstat(fd, &filestatus);
    if(err) return -1;
    return filestatus.st_size;
}

static I32 compile_shader_file(GLenum type, const char *path, U32 *shader_out)
{
    // Read shader file to buffer
    I32 file = open(path, O_RDONLY);
    if(file == -1) return JK_ERROR_FILEOPEN;
    I64 file_size = fsize(file);
    char *source = (char *)malloc(file_size);
    read(file, source, file_size);
    close(file);

    I32 compilation_success;
    U32 shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&source, (const I32 *)&file_size);
    free(source);
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

void delete_shader(U32 program)
{
    glDeleteProgram(program);
}

/*

////////////////////////////////////////
// simple type uniforms
void uniform_i32(U32 program, const char *uniform_name, I32 value) {
    glUniform1i(glGetUniformLocation(program, uniform_name), value);
}
void uniform_f32(U32 program, const char *uniform_name, F32 value) {
    glUniform1f(glGetUniformLocation(program, uniform_name), value);
}
void uniform_f32_num3(U32 program, const char *uniform_name, F32 first, F32 second, F32 third) {
    glUniform3f(glGetUniformLocation(program, uniform_name), first, second, third);
}
void uniform_f32_vec3(U32 program, const char *uniform_name, const F32 *values) {
    glUniform3fv(glGetUniformLocation(program, uniform_name), 1, values);
}
void uniform_f32_mat4x4(U32 program, const char *uniform_name, const F32 *values) {
    glUniformMatrix4fv(glGetUniformLocation(program, uniform_name), 1, GL_FALSE, values);
}

////////////////////////////////////////
// glm types
void uniform_glm_vec3(U32 program, const char *uniform_name, const glm::vec3 *vector) {
    glUniform3fv(glGetUniformLocation(program, uniform_name), 1, glm::value_ptr(*vector));
}
void uniform_glm_mat4(U32 program, const char *uniform_name, const glm::mat4 *matrix) {
    glUniformMatrix4fv(glGetUniformLocation(program, uniform_name), 1, GL_FALSE, glm::value_ptr(*matrix));
}

*/
