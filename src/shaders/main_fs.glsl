#version 430 core

out vec4 frag_color;

in vec3 frag_pos;
in vec3 frag_norm;
in vec2 frag_texcoord;

uniform vec3 camera_pos;

// uniform vec3 base_color;
// uniform sampler2D tex_base_color;
// uniform sampler2D tex_metallic_roughness;

#define jm_pow2(x) ((x) * (x))

#define jm_pow4(x) ((x) * (x) * (x) * (x))

void main() {
    const float pi = 3.14159;
    const float pi_half = pi/2.0;
    const float one_over_pi = 1.0/pi;

    const float metallic = 0.9;
    const float roughness = 0.5;

    const vec3 light_pos = vec3(-2.0, 2.0, 4.0);
    const vec3 light_color = vec3(10.0);
    const vec3 color = vec3(0.8, 0.7, 0.2);

    vec3 n = frag_norm;
    vec3 l = normalize(light_pos - frag_pos);
    vec3 v = normalize(camera_pos - frag_pos);
    vec3 h = normalize(l + v);

    float dot_nh = max(dot(n, h), 0.0);
    float dot_nl = max(dot(n, l), 0.0);

    // jdk: attenuation and incidence angle
    float ldist = distance(light_pos, frag_pos);
    vec3 light_power_in = light_color/(ldist*ldist) * dot_nl;

    vec3 specular_color = mix(vec3(0.04) /*dielectric*/, color, metallic);
    vec3 diffuse_color = (1.0 - metallic) * color;

    float rough = 0.1 + 0.9 * roughness;
    float gloss = (1.0 - jm_pow4(rough));
    vec3 specular = one_over_pi * rough / (1.0 - (dot_nh*dot_nh) * gloss) * specular_color;
    vec3 diffuse = one_over_pi * diffuse_color;
    vec3 ambient = 0.03 * color;
    vec3 result_color = (diffuse + specular) * light_power_in + ambient;

    frag_color = vec4(result_color, 1.0);
}
