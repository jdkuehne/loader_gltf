#version 430 core

out vec4 frag_color;

in vec3 frag_pos;
in vec3 frag_norm;

uniform vec3 camera_pos = vec3(2.0, 2.0, 3.0);
uniform vec3 light_pos = vec3(-3.0, 2.0, 4.0);
uniform vec3 light_color = vec3(10.0, 9.8, 9.5);

uniform vec3 base_color;
uniform float factor_metallic = 0.9;
uniform float factor_roughness = 0.9;
uniform sampler2D tex_base_color;
uniform sampler2D tex_metallic_roughness;

void main() {
    // vec3 res_color = texture(tex_base_color, vec2(
    const vec3 color = vec3(0.6, 0.3, 0.1);

    vec3 L = normalize(light_pos - frag_pos);
    // TODO(jdk): double normalized?
    vec3 N = normalize(frag_norm);
    vec3 V = normalize(camera_pos - frag_pos);
    vec3 H = normalize(L + V);

    float ldistance = length(L);
    float attenuation = 1.0/(ldistance*ldistance);
    vec3 light = light_color * attenuation;

    // jdk: fresnel reflection at normal incidence ((n0 - n1)*(n0 + n1))^2
    // e.g. polyethylene: 1.5 => 0.5/2.5 = 0.2*0.2 = 0.04
    vec3 r0_dielectric = vec3(0.04);
    // jdk: the color of a metal is pure specular reflection => fresnel term is color
    vec3 r0 = mix(r0_dielectric, color, metallic);

    float one_over_pi = 1.0/3.14159;

    // jdk: lambertian diffuse
    vec3 diffuse = max(dot(L, N), 0) * color * light * one_over_pi;

    float NdotH = dot(N, H);
    float alpha = factor_roughness * factor_roughness;
    float alpha_square = alpha * alpha;
    float normal_distribution = one_over_pi * alpha_square /
	pow(NdotH*NdotH * (alpha_square - 1.0) + 1.0, 2.0);

    // jdk: UE4 does additional approximation, dot(L, H) because
    // fresnel calculates "perfect reflection" => halfway is the normal,
    // and the normal distribution then multiplies by how often we actually
    // get this case with microfacets??
    vec3 fresnel = r0 + (1 - r0) * pow(1 - max(dot(L, H), 0), 5);





    float specular_exponent = floor(512.0 * (1.0 - pow(factor_roughness, 2.0)));
    float specular = pow(max(dot(halfway, frag_norm), 0), specular_exponent);

    vec3 specular_dielectric_color = vec3(specular);
    vec3 specular_metallic_color = specular * color;
    vec3 specular_color = mix(specular_dielectric_color, specular_metallic_color, factor_metallic);

    vec3 ambient = vec3(0.03) * color;

    vec3 result_color = diffuse + ambient;

    frag_color = vec4(result_color, 1.0);
}
