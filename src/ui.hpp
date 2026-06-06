#ifndef UI_H
#define UI_H

#include "base/core.hpp"
#include "base/str.hpp"
#include "base/vec.hpp"

#include "shader.hpp"

#include "ext/glad/gl.h"
#include "ext/stb_easy_font.h"

typedef struct TextObject {
    // jdk: in pixels, but scales with font size
    U64 w, h;
    U32 vbo, ebo, vao;
    U64 num_indices;
} TextObject;

typedef struct TextShaderInfo {
    U32 program;

    U32 location_window_size;
    U32 location_fg_color;
    U32 location_upper_left;
    U32 location_font_scale;
} TextShaderInfo;

TextObject *new_text_object(Str8 text);
void delete_text_object(TextObject *obj);
void draw_textbox_no_background(const TextObject *text,
				Vec3 color, F32 font_scale,
				U64 offset_x, U64 offset_y,
				U64 window_width, U64 window_height);

#endif
