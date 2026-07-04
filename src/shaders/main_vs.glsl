#version 430 core

layout (location = 0) in vec3 attrib_pos;
layout (location = 1) in vec3 attrib_norm;
layout (location = 2) in vec3 attrib_tangent;
layout (location = 3) in vec2 attrib_texcoord0;
// tangent
layout (location = 7) in uvec4 attrib_joints;
layout (location = 9) in vec4 attrib_weights;

layout(std140) struct OptionalSampler2D {
    bool has_texture;
    sampler2D sampler;
};

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

uniform bool has_skin;
uniform mat4 joint_matrices[64];

uniform OptionalSampler2D morph_attrib_pos;
uniform OptionalSampler2D morph_attrib_norm;
uniform OptionalSampler2D morph_attrib_tangent;
uniform OptionalSampler2D morph_attrib_texcoord0;
// jdk: spec allows for color as well...
// and more texcoords n' stuff

uniform float morph_weights[32];

out vec3 frag_pos;
out vec3 frag_norm;
out vec2 frag_texcoord;

vec4 morph_from_texture(OptionalSampler2D morph_attrib) {
    vec4 value = vec4(0.0);
    if(morph_attrib.has_texture) {
	for(int i = 0; i < int(textureSize(morph_attrib.sampler, 0).y); ++i) {
	    value += morph_weights[i] * texelFetch(morph_attrib.sampler, ivec2(gl_VertexID, i), 0);
	}
    }
    return value;
}

void main() {
    mat4 animated_world = !has_skin ? world :
	attrib_weights.x * joint_matrices[attrib_joints.x] +
	attrib_weights.y * joint_matrices[attrib_joints.y] +
	attrib_weights.z * joint_matrices[attrib_joints.z] +
	attrib_weights.w * joint_matrices[attrib_joints.w];

    vec3 pos = attrib_pos + morph_from_texture(morph_attrib_pos).rgb;
    vec3 norm = attrib_norm + morph_from_texture(morph_attrib_norm).rgb;
    vec3 tangent = attrib_tangent + morph_from_texture(morph_attrib_tangent).rgb;
    vec2 texcoord0 = attrib_texcoord0 + morph_from_texture(morph_attrib_texcoord0).rg;

    frag_norm = normalize(transpose(inverse(mat3(animated_world))) * norm);
    frag_pos = (animated_world * vec4(pos, 1.0)).xyz;
    frag_texcoord = texcoord0;
    gl_Position = projection * view * animated_world * vec4(pos, 1.0);
}
