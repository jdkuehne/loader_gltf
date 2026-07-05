#version 430 core

out vec4 frag_color;

in vec3 frag_pos;
in vec3 frag_norm;
in vec2 frag_texcoord;

uniform vec3 camera_pos;

layout (std140, binding = 1) uniform UBO_FS0 {
    vec3 albedo_factor;
    float metallic_factor;
    float roughness_factor;
    bool has_albedo_texture;
    bool has_metallic_channel;
    bool has_roughness_channel;
};

// uniform vec3 albedo_factor = vec3(0.6);
uniform sampler2D albedo_texture;
// uniform float roughness_factor = 0.5;
// uniform float metallic_factor = 0.5;
uniform sampler2D metallic_roughness_texture;


#define jm_pow2(x) ((x) * (x))

#define jm_pow4(x) ((x) * (x) * (x) * (x))

void main() {
    float metallic = (has_metallic_channel ? texture(metallic_roughness_texture, frag_texcoord).x : 1.0) * metallic_factor;
    float roughness = (has_roughness_channel ? texture(metallic_roughness_texture, frag_texcoord).y : 1.0) * roughness_factor;
    vec3 albedo = (has_albedo_texture ? texture(albedo_texture, frag_texcoord).rgb : vec3(1.0)) * albedo_factor;
    // vec3 albedo = vec3(0.0);
    const float pi = 3.14159;
    const float pi_half = pi/2.0;
    const float one_over_pi = 1.0/pi;

    const vec3 light_pos = vec3(7.0, 5.0, -7.0);
    const vec3 light_color = vec3(35.0);

    vec3 n = frag_norm;
    vec3 l = normalize(light_pos - frag_pos);
    vec3 v = normalize(camera_pos - frag_pos);
    vec3 h = normalize(l + v);

    float dot_nh = max(dot(n, h), 0.0);
    float dot_nl = max(dot(n, l), 0.0);

    // jdk: attenuation and incidence angle
    float ldist = distance(light_pos, frag_pos);
    vec3 light_power_in = light_color/(ldist*ldist) * dot_nl;

    vec3 specular_color = mix(vec3(0.04) /*dielectric*/, albedo, metallic);
    vec3 diffuse_color = (1.0 - metallic) * albedo;

    float s_factor = 1.5 * (1.0 - roughness);
    float s_exponent = 1 - jm_pow4(roughness);
    float s_offset = roughness * one_over_pi;
    vec3 specular = (s_factor * pow(dot_nh, s_exponent) + s_offset) * specular_color;
    vec3 diffuse = one_over_pi * diffuse_color;
    vec3 ambient = 0.05 * albedo;
    vec3 result_color = (diffuse + specular) * light_power_in + ambient;

    frag_color = vec4(result_color, 1.0);
}
