#version 430 core

out vec4 frag_color;

in vec3 frag_pos;
in vec3 frag_norm;

const vec3 light_pos = vec3(2.0, -1.0, 8.0);
uniform vec3 color;

void main() {
    const float ambient = 0.2;
    float diffuse = max(dot(normalize(light_pos - frag_pos), frag_norm), 0);
    frag_color = (ambient + diffuse) * vec4(color, 1.0);
}
