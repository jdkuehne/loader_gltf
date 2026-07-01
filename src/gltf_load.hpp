#ifndef GLTF_LOAD_H
#define GLTF_LOAD_H

#include "jbase.hpp"
#include "jmath.hpp"
#include "transform.hpp"
#include "containers.hpp"
#include "shader.hpp"
#include "ext/cgltf.h"
#include "ext/glad/gl.h"

#define JK_NUM_MORPH_ATTRIBS 7

using AttribGroup = Stack<cgltf_attribute *>;

struct AnimStep {
    uint64_t i0, i1;
    float interpolation_factor;
};

struct PrimMeta {
    // jdk: this address is used as key to find corresponding data
    cgltf_primitive *key;
    uint32_t vbo, ebo, vao;
    uint64_t indices_count;
    GLenum indices_type;
    // jdk: one texture per attrib, second dimension are the other morph targets
    uint32_t morph_attribute_textures[JK_NUM_MORPH_ATTRIBS];
    cgltf_pbr_metallic_roughness *metallic_roughness;
    uint32_t base_color_texture_id;
};

struct ChannelMeta {
    cgltf_animation_path_type target;
    cgltf_interpolation_type interpolation;
    // TODO(jdk): what to do if weights?
    uint64_t num_frames;
    Str8 input;
    Str8 output;
};

struct AnimMeta {
    Str8 name;
    // @TODO(jdk): should probably be a stack with cap 4
    Link<ChannelMeta> *channel_first;
};

#define JK_NODE_IS_NOT_SKIN_JOINT (-1)
struct SkinJointsData {
    cgltf_node **joints;
    Mat4 *skeleton_matrices;
    uint64_t len;
};

struct NodeMeta {
    cgltf_node *key;

    bool has_skin;
    bool has_normal_mapping;

    Mat4 static_matrix;
    Mat4 matrix;
    Mat4 world_matrix;

    Slice<float> morph_weights;

    SkinJointsData skin_data;
    Slice<Mat4> inverse_bind_matrices;

    Stack<PrimMeta> primitives_meta;
    Link<AnimMeta> *anim_first;
};

struct GLTFBin {
    Str8 path;
    Str8 data;
};

// jdk: here all the acquired data is stored
struct GLTFModel {
    List<NodeMeta> nodes_meta;
    List<GLTFBin> bin_files;
    cgltf_data *data;
    struct {
	Str8 name_current;
	uint64_t time_ms;
    } anim;
};

// jdk: these are the input parameters for all gltf related functions
struct GLTFLoadParams {
    Str8 file_dir;
    Str8 file_name;
    cgltf_options options;
    Str8 bin_dir;
};

GLTFModel *gltf_load(GLTFLoadParams *params);
// TODO(jdk): implement destroy later
void gltf_destroy(GLTFModel *model);

void gltf_animate(GLTFModel *model);
void gltf_draw(GLTFModel *model, Mat4 base_matrix);

#endif // GLTF_LOAD_H
