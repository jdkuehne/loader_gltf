#version 430 core

out vec4 frag_color;

uniform vec3 fg_color;

void main() {
    frag_color = vec4(fg_color, 1.0);
}
