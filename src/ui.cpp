#include "ui.hpp"

static TextShaderInfo *shader_info = NULL;

// NOTE(jdk): new indicates that result is heap allocated via the context's heap_alloc
TextObject *new_text_object(Str8 text) {
    TextObject *result = alloc<TextObject>();
    char *text_cstr = cstr_from_str8(text, &temp_allocator);

    uint64_t bufsize = text.len * 512;
    void *buffer = alloc<float>(bufsize, &temp_allocator);
    uint64_t num_quads = stb_easy_font_print(0.f, 0.f, text_cstr, NULL, buffer, bufsize);
    uint64_t num_indices = num_quads * 6;
    uint64_t indices_bufsize = num_indices * sizeof(uint32_t);
    uint32_t *indices_buffer = alloc<uint32_t>(indices_bufsize, &temp_allocator);
    // jdk: fill a index buffer that draws as if it was GL_QUADS (deprecated opengl feature)
    for(uint64_t i = 0; i < num_quads; ++i) {
	uint64_t i_offset = 6 * i;
	uint64_t val_offset = 4 * i;
	indices_buffer[i_offset + 0] = 0 + val_offset;
	indices_buffer[i_offset + 1] = 1 + val_offset;
	indices_buffer[i_offset + 2] = 2 + val_offset;
	indices_buffer[i_offset + 3] = 0 + val_offset;
	indices_buffer[i_offset + 4] = 2 + val_offset;
	indices_buffer[i_offset + 5] = 3 + val_offset;
    }

    uint32_t vbo, ebo, vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, bufsize, buffer, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_bufsize, indices_buffer, GL_STATIC_DRAW);

    uint64_t stride = 3 * sizeof(float) + 4 * sizeof(uint8_t);
    const void *offset_position = (const void *)NULL;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, offset_position);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // jdk: setup TextObject
    // NOTE(jdk): technically -2 would remove the space of the last char, making it actually
    // centered, but i kinda prefer just to go with what the library provides
    result->w = stb_easy_font_width(text_cstr); // -2
    result->h = stb_easy_font_height(text_cstr);
    result->vbo = vbo;
    result->ebo = ebo;
    result->vao = vao;
    result->num_indices = num_indices;
    return result;
}

void delete_text_object(TextObject *obj) {
    if(obj) {
	glDeleteBuffers(1, &obj->vbo);
	glDeleteBuffers(1, &obj->ebo);
	glDeleteVertexArrays(1, &obj->vao);
	release(obj);
    }
}

// jdk: offsets are in pixels
void draw_textbox_no_background(const TextObject *text,
				Vec3 color, float font_scale,
				uint64_t offset_x, uint64_t offset_y,
				uint64_t window_width, uint64_t window_height) {
    if(!shader_info) {
	TextShaderInfo *p = alloc<TextShaderInfo>();
	p->program = create_shader_vf("./src/shaders/text_vs.glsl",
		"./src/shaders/text_fs.glsl");
	p->location_fg_color    = glGetUniformLocation(p->program, "fg_color");
	p->location_font_scale  = glGetUniformLocation(p->program, "font_scale");
	p->location_upper_left  = glGetUniformLocation(p->program, "upper_left");
	p->location_window_size = glGetUniformLocation(p->program, "window_size");
	shader_info = p;
    }

    glUseProgram(shader_info->program);
    glBindVertexArray(text->vao);
    glUniform3fv(shader_info->location_fg_color, 1, color.v);
    glUniform1f (shader_info->location_font_scale, font_scale);
    glUniform2f (shader_info->location_upper_left, offset_x, offset_y);
    glUniform2f (shader_info->location_window_size, (float)window_width, (float)window_height);
    glDrawElements(GL_TRIANGLES, text->num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
