#ifndef GLTF_LOAD_H
#define GLTF_LOAD_H

#include "base/core.hpp"
#include "base/str.hpp"
#include "base/mat.hpp"
#include "base/mem/allocator.hpp"
#include "base/os/file.hpp"
#include "base/containers.hpp"

#include "ext/cgltf.h"

#include "ext/glad/gl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JK_NUM_MORPH_ATTRIBS 7

using AttribGroup = Stack<cgltf_attribute *>;

struct AnimStep {
    U64 i0, i1;
    F32 interpolation_factor;
};


struct PrimMeta {
    // jdk: this address is used as key to find corresponding data
    cgltf_primitive *key;
    U32 vbo, ebo, vao;
    U64 indices_count;
    GLenum indices_type;
    // jdk: one texture per attrib, second dimension are the other morph targets
    U32 morph_attribute_textures[JK_NUM_MORPH_ATTRIBS];
};

// NOTE(jdk): Channel and AnimMeta are linked lists because there's no proper way to know
// how big the allocation should be, so being flexible with the list's size makes sense.

struct ChannelMeta {
    cgltf_animation_path_type target;
    cgltf_interpolation_type interpolation;
    // TODO(jdk): what to do if weights?
    U64 num_frames;
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
    U64 len;
};

struct NodeMeta {
    cgltf_node *key;

    B8 has_skin;
    B8 has_normal_mapping;

    Mat4 static_matrix;
    Mat4 matrix;
    Mat4 world_matrix;

    Slice<F32> morph_weights;

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
	U32 program;
	U32 location_world_matrix;
	U32 location_joint_matrices;
	U32 location_has_skin;
	Mat4 base_transform;
    } draw;
    struct {
	Str8 name_current;
	U64 time_ms;
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
void gltf_draw(GLTFModel *model);

#endif // GLTF_LOAD_H
