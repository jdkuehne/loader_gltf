#version 430 core

out vec4 frag_color;

in vec3 frag_pos;
in vec3 frag_norm;
in vec2 frag_texcoord;

uniform vec3 camera_pos;

// uniform vec3 base_color;
// uniform sampler2D tex_base_color;
// uniform sampler2D tex_metallic_roughness;

float square(float x) {
    return x*x;
}

void main() {
    const float one_over_pi = 1.0/3.14159;

    const float metallic = 0.0;
    const float roughness = 0.1;

    const vec3 light_pos = vec3(-2.0, 2.0, 5.0);
    const vec3 light_color = vec3(2.0);
    const vec3 color = vec3(0.79, 0.78, 0.8);

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
    float ldist = length(l);
    vec3 light_power_in = light_color/(ldist*ldist) * dot_nl;

    // jdk: trowbridge-reitz makes the distribution "hyperbolic" which looks pretty smooth
    float rough_param = square(roughness * roughness); // jdk: pow4 remapping
    float dot_nh_square = dot_nh * dot_nh;
    float nd_denom = square(dot_nh_square * (1.0 - rough_param) - 1.0);
    float d = rough_param * one_over_pi / nd_denom;

    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float g0 = dot_nv / (dot_nv * (1.0 - k) + k);
    float g1 = dot_nl / (dot_nl * (1.0 - k) + k);
    float g = g0 * g1;

    // jdk: fresnel schlick
    vec3 f0_dielectric = vec3(0.04);
    vec3 f0 = mix(f0_dielectric, color, metallic);
    vec3 f = f0 + (1.0 - f0) * pow(1.0 - dot_lh, 5.0);

    // TODO(jdk): why /(cos_alpha*cos_beta)??? isn't this shit solved already via fresnel?
    vec3 ks = d*g*f / (4.0 * dot_nv*dot_nl + 0.001);
    vec3 kd = (1.0 - metallic) * one_over_pi *color;
    vec3 light_power_out = (ks + kd) * light_power_in;

    vec3 ambient = vec3(0.03) * color;

    vec3 result_color = light_power_out + ambient;
    frag_color = vec4(result_color, 1.0);
}
// NOTES:

// jdk: fresnel reflection at normal incidence ((n0 - n1)*(n0 + n1))^2
// e.g. polyethylene: 1.5 => 0.5/2.5 = 0.2*0.2 = 0.04
// metalls only reflect => reflection is color

// jdk: dot(L, H) because
// fresnel calculates "perfect reflection" => halfway is the normal,
// and the normal distribution then multiplies by how often we actually
// get this case with microfacets??
