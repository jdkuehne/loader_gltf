#ifndef UI_H
#define UI_H

#include "jbase.hpp"
#include "shader.hpp"
#include "ext/glad/gl.h"
#include "ext/stb_easy_font.h"

typedef struct TextObject {
    // jdk: in pixels, but scales with font size
    uint64_t w, h;
    uint32_t vbo, ebo, vao;
    uint64_t num_indices;
} TextObject;

typedef struct TextShaderInfo {
    uint32_t program;

    uint32_t location_window_size;
    uint32_t location_fg_color;
    uint32_t location_upper_left;
    uint32_t location_font_scale;
} TextShaderInfo;

TextObject *new_text_object(Str8 text);
void delete_text_object(TextObject *obj);
void draw_textbox_no_background(const TextObject *text,
				Vec3 color, float font_scale,
				uint64_t offset_x, uint64_t offset_y,
				uint64_t window_width, uint64_t window_height);

#endif
