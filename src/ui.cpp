#include "ui.hpp"

static TextShaderInfo *shader_info = NULL;

// NOTE(jdk): new indicates that result is heap allocated via the context's heap_alloc
TextObject *new_text_object(Context *context, Str8 text) {
    TextObject *result = (TextObject *)context->heap_alloc(sizeof(TextObject));
    char *text_cstr = cstr_copy_from_str8(context->frame_arena, text);

    U64 bufsize = text.len * 512;
    void *buffer = arena_alloc(context->frame_arena, bufsize,
					   JK_AlignOf(F32));
    U64 num_quads = stb_easy_font_print(0.f, 0.f, text_cstr, NULL, buffer, bufsize);
    U64 num_indices = num_quads * 6;
    U64 indices_bufsize = num_indices * sizeof(U32);
    U32 *indices_buffer = arena_alloc<U32>(context->frame_arena, indices_bufsize);
    // jdk: fill a index buffer that draws as if it was GL_QUADS (deprecated opengl feature)
    for(U64 i = 0; i < num_quads; ++i) {
	U64 i_offset = 6 * i;
	U64 val_offset = 4 * i;
	indices_buffer[i_offset + 0] = 0 + val_offset;
	indices_buffer[i_offset + 1] = 1 + val_offset;
	indices_buffer[i_offset + 2] = 2 + val_offset;
	indices_buffer[i_offset + 3] = 0 + val_offset;
	indices_buffer[i_offset + 4] = 2 + val_offset;
	indices_buffer[i_offset + 5] = 3 + val_offset;
    }

    U32 vbo, ebo, vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, bufsize, buffer, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_bufsize, indices_buffer, GL_STATIC_DRAW);

    U64 stride = 3 * sizeof(F32) + 4 * sizeof(U8);
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

// jdk: offsets are in pixels
void draw_textbox_no_background(Context *context, const TextObject *text,
				Vec3 color, F32 font_scale,
				U64 offset_x, U64 offset_y,
				U64 window_width, U64 window_height) {
    if(!shader_info) {
	TextShaderInfo *p = arena_alloc<TextShaderInfo>(context->persistent_arena);
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
    glUniform2f (shader_info->location_window_size, (F32)window_width, (F32)window_height);
    glDrawElements(GL_TRIANGLES, text->num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
