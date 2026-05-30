#version 430 core

layout (location = 0) in vec3 attrib_pos;

uniform float font_scale;
uniform vec2 upper_left;
uniform vec2 window_size;

// => x/w * 2 - 1
void main() {
    vec2 pos = attrib_pos.xy;
    // jdk: apply font scaling, pos is always from 0, 0 because as defined in ui.h/.c
    pos = upper_left + pos * font_scale;
    pos = (pos / window_size * 2.0 - vec2(1.0));
    pos = pos * vec2(1.0, -1.0);
    gl_Position = vec4(pos, 0.0, 1.0);
}
