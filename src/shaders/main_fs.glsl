#version 430 core

out vec4 frag_color;

in vec3 frag_pos;
in vec3 frag_norm;
in vec2 frag_texcoord;

uniform vec3 camera_pos;

// uniform vec3 base_color;
// uniform sampler2D tex_base_color;
// uniform sampler2D tex_metallic_roughness;

float pow2(float x) {
    return x*x;
}

float pow4(float x) {
    return x*x * x*x;
}

void main() {
    const float one_over_pi = 1.0/3.14159;

    const float metallic = 0.0;
    const float roughness = 0.4;

    const vec3 light_pos = vec3(-2.0, 2.0, 4.0);
    const vec3 light_color = vec3(5.0);
    const vec3 color = vec3(0.79, 0.65, 0.34);

    vec3 l = normalize(light_pos - frag_pos);
    vec3 n = frag_norm;
    vec3 v = normalize(camera_pos - frag_pos);
    vec3 h = normalize(l + v);

    // jdk: theta = halfway/normal, alpha = view/normal, beta = light/halfway (fresnel)
    float dot_nh = max(dot(n, h), 0.0);
    float dot_nv = max(dot(n, v), 0.0);
    float dot_nl = max(dot(n, l), 0.0);
    float dot_lh = max(dot(l, h), 0.0);

    // jdk: attenuation
    float ldist = distance(light_pos, frag_pos);
    vec3 light_power_in = light_color/(ldist * ldist) * dot_nl;

    float diffuse_modifier = 0.4f;
    float specular_modifier = 0.2f;
    float ambient_modifier = 0.03f;

    // jdk: based on trowbridge reitz distribution
    // @TODO(jdk): try out more stuff with these functions (why not 1 as nominator?)
    vec3 specular_color_dielectric = vec3(0.04);
    vec3 specular_color = mix(specular_color_dielectric, color, metallic);
    float a = pow4(roughness);
    // @TODO(jdk): for now this is good enough
    vec3 specular = (specular_modifier * a)/pow2(pow2(dot_nh)*(1-a) - 1) * specular_color;
    vec3 diffuse = (1.0 - metallic) * diffuse_modifier * color;
    vec3 ambient = vec3(0.03) * color;
    vec3 result_color = (diffuse + specular) * light_power_in + ambient;

    frag_color = vec4(result_color, 1.0);
}
