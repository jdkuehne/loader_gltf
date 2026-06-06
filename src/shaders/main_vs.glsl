#version 430 core

layout (location = 0) in vec3 attrib_pos;
layout (location = 1) in vec3 attrib_norm;
layout (location = 2) in vec3 attrib_tangent;
layout (location = 3) in vec2 attrib_texcoord0;
// tangent
// tex coords
layout (location = 7) in uvec4 attrib_joints;
layout (location = 9) in vec4 attrib_weights;

// // @TODO(jdk): uniform int num_morph_targets to make it explicit? (already visible in texwidth)
const int JK_NUM_MORPH_ATTRIBS = 7;

const int JK_MORPH_POSITION   = 0;
const int JK_MORPH_NORMAL     = 1;
const int JK_MORPH_TANGENT    = 2;
const int JK_MORPH_TEXCOORD_0 = 3;
const int JK_MORPH_TEXCOORD_1 = 4;
const int JK_MORPH_COLOR_0    = 5;
const int JK_MORPH_COLOR_1    = 6;


uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

uniform bool has_skin;
uniform mat4 joint_matrices[64];

uniform sampler2D morph_texture0; // jdk: position
uniform sampler2D morph_texture1; // jdk: normals
uniform sampler2D morph_texture2; // jdk: tangents
uniform sampler2D morph_texture3; // jdk: texcoord0
uniform sampler2D morph_texture4; // ...
uniform sampler2D morph_texture5; // jdk: color0
uniform sampler2D morph_texture6; // ...

uniform float morph_weights[64];

out vec3 frag_pos;
out vec3 frag_norm;
out vec2 frag_texcoord;

void main() {
    mat4 skin_mat =
	attrib_weights.x * joint_matrices[attrib_joints.x] +
	attrib_weights.y * joint_matrices[attrib_joints.y] +
	attrib_weights.z * joint_matrices[attrib_joints.z] +
	attrib_weights.w * joint_matrices[attrib_joints.w];
    mat4 animated_world = (has_skin) ? world * skin_mat : world;

    vec3 pos = attrib_pos;
    // @TODO(jdk): find out why looks wrong (maybe a sampling issue)
    vec2 morph0_texsize = textureSize(morph_texture0, 0);
    for(int i = 0; i < int(morph0_texsize.y); ++i) {
	pos += morph_weights[i] * texelFetch(morph_texture0, ivec2(gl_VertexID, i), 0).rgb;
    }

    vec3 norm = attrib_norm;
    for(int i = 0; i < int(textureSize(morph_texture1, 0).y); ++i) {
	norm += morph_weights[i] * texelFetch(morph_texture1, ivec2(gl_VertexID, i), 0).rgb;
    }

    frag_norm = normalize(transpose(inverse(mat3(animated_world))) * norm);
    frag_pos = vec3(animated_world * vec4(pos, 1.0));
    frag_texcoord = attrib_texcoord0;
    gl_Position = proj * view * animated_world * vec4(pos, 1.0);
}
