#include "gltf_load.hpp"

// jdk: search conditions
static bool match_prim(PrimMeta *x, cgltf_primitive *key) { return (x->key == key) ? true : false; }
static bool match_node(NodeMeta *x, cgltf_node *key) { return (x->key == key) ? true : false; }
static bool bin_data_file_has_path(GLTFBin *x, Str8 path) { return str8_equal(x->path, path); }
static bool match_anim(AnimMeta *x, Str8 anim_name) { return str8_equal(x->name, anim_name); }
static bool match_channel(ChannelMeta *x, cgltf_animation_path_type key) { return x->target == key; }

// jdk: unused
template<typename ElemT, typename CompT>
ElemT *arr_find(ElemT *arr, size_t size, bool (*comp_fn)(ElemT *, CompT), CompT comp_val) {
    for(size_t i = 0; i < size; ++i) {
	if(comp_fn(&arr[i], comp_val)) {
	    return &arr[i];
	}
    }
}

static uint64_t cgltf_component_type_size(cgltf_component_type x) {
    struct { cgltf_component_type type; uint64_t size; } static const map[] = {
	{cgltf_component_type_r_8, sizeof(int8_t)}, {cgltf_component_type_r_8u, sizeof(uint8_t)},
	{cgltf_component_type_r_16, sizeof(int16_t)}, {cgltf_component_type_r_16u, sizeof(uint16_t)},
	{cgltf_component_type_r_32u, sizeof(uint32_t)}, {cgltf_component_type_r_32f, sizeof(float)},
    };
    for(uint64_t i = 0; i < jm_arrlen(map); ++i) {
	if(map[i].type == x)
	    return map[i].size;
    }
    assert(0 && "invalid component type");
    return 0; // jdk: because msvc doesn't recognise assert 0 as exit
}

static GLenum gl_enum_from_cgltf_component_type(cgltf_component_type x) {
    struct { cgltf_component_type comp_type; GLenum gl_comp_type; } static const map[] = {
	{cgltf_component_type_r_8, GL_BYTE}, {cgltf_component_type_r_8u, GL_UNSIGNED_BYTE},
	{cgltf_component_type_r_16, GL_SHORT}, {cgltf_component_type_r_16u, GL_UNSIGNED_SHORT},
	{cgltf_component_type_r_32u, GL_UNSIGNED_INT}, {cgltf_component_type_r_32f, GL_FLOAT},
    };
    for(uint64_t i = 0; i < jm_arrlen(map); ++i) {
	if(map[i].comp_type == x) {
	    return map[i].gl_comp_type;
	}
    }
    assert(0 && "invalid component type");
    return 0;
}

// // @TODO(jdk): just use the one from the library
static uint64_t cgltf_type_component_count(cgltf_type x) {
    struct { cgltf_type type; uint64_t count; } static const map[] = {
	{cgltf_type_scalar, 1},
	{cgltf_type_vec2, 2}, {cgltf_type_vec3, 3}, {cgltf_type_vec4, 4},
	{cgltf_type_mat2, 4}, {cgltf_type_mat3, 9}, {cgltf_type_mat4, 16},
    };
    for(uint64_t i = 0; i < jm_arrlen(map); ++i) {
	if(map[i].type == x) {
	    return map[i].count;
	}
    }
    assert(0 && "cannot get component count of invalid type");
    return 0;
}

static GLenum cgltf_type_tex_format(cgltf_type x) {
    struct { cgltf_type type; GLenum format; } static const map[] = {
	{cgltf_type_scalar, GL_RED}, {cgltf_type_vec2, GL_RG},
	{cgltf_type_vec3, GL_RGB}, {cgltf_type_vec4, GL_RGBA},
    };
    for(uint64_t i = 0; i < jm_arrlen(map); ++i) {
	if(x == map[i].type) {
	    return map[i].format;
	}
    }
    assert(0 && "cannot get pixel format for cgltf_type");
    return 0;
}

static int get_attrib_location(Str8 attrib_name) {
    struct { const char *name; int location; } static const map[] = {
	{"POSITION", 0}, {"NORMAL", 1}, {"TANGENT", 2},
	{"TEXCOORD_0", 3}, {"TEXCOORD_1", 4}, {"COLOR_0", 5}, {"COLOR_1", 6},
	{"JOINTS_0", 7}, {"JOINTS_1", 8}, {"WEIGHTS_0", 9}, {"WEIGHTS_1", 10},
    };
    for(uint64_t i = 0; i < jm_arrlen(map); ++i) {
	if(str8_equal(attrib_name, str8(map[i].name))) {
	    return map[i].location;
	}
    }
    exit(JM_ERROR_UNKNOWN_PRIMITIVE_ATTRIB);
}

static int get_morph_attrib_location(Str8 attrib_name) {
    struct { const char *name; int location; } static const map[] = {
	{"POSITION", 0}, {"NORMAL", 1}, {"TANGENT", 2},
	{"TEXCOORD_0", 3}, {"TEXCOORD_1", 4}, {"COLOR_0", 5}, {"COLOR_1", 6},
    };
    for(uint64_t i = 0; i < jm_arrlen(map); ++i) {
	if(str8_equal(attrib_name, str8(map[i].name))) {
	    return map[i].location;
	}
    }
    exit(JM_ERROR_UNKNOWN_PRIMITIVE_ATTRIB);
}

// // @TODO(jdk): no handling for ./... vs ... paths
// jdk: maybe introduce reference counting for files?
static Str8 get_view_bin_data(GLTFModel *model, cgltf_buffer_view *view, Str8 dir) {
    dir = str8_dir_finish_with_slash(dir, &temp_allocator);
    Str8 path = str8_cat(dir, str8(view->buffer->uri), &temp_allocator);
    GLTFBin *entry = list_find(&model->bin_files, &bin_data_file_has_path, path);
    if(entry) {
	return str8_substr(entry->data, view->offset, view->size);
    } else {
	Str8 buffer_data = file_read_full_to_str8(path);
	Str8 view_data = str8_substr(buffer_data, view->offset, view->size);
	GLTFBin new_entry = {str8_copy(path), buffer_data};
	list_push(&model->bin_files, new_entry);
	return view_data;
    }
}

static Str8 get_anim_sampler_accessor_data(GLTFModel *model,
	cgltf_accessor *accessor /*input or output*/, Str8 bin_dir) {
    cgltf_buffer_view *view = accessor->buffer_view;
    Str8 view_data = get_view_bin_data(model, view, bin_dir);
    uint64_t offset = accessor->offset;
    uint64_t component_count = cgltf_type_component_count(accessor->type);
    uint64_t component_size = cgltf_component_type_size(accessor->component_type);
    uint64_t size = accessor->count * component_count * component_size;
    return str8_substr(view_data, offset, size);
}

static inline uint64_t step_time_ms(float *buf_s, uint64_t i) { return (uint64_t)(buf_s[i] * 1000.f); }
static AnimStep get_anim_step(ChannelMeta *channel_meta, uint64_t time_ms) {
    float *time_buffer_s = (float *)channel_meta->input.ptr;
    uint64_t num_frames = channel_meta->num_frames;
    uint64_t time_min_ms = step_time_ms(time_buffer_s, 0);
    uint64_t time_max_ms = step_time_ms(time_buffer_s, num_frames - 1);
    uint64_t time_ms_mapped = time_ms % (time_max_ms - time_min_ms) + time_min_ms;
    for(uint64_t i = 0; i < num_frames; ++i) {
	uint64_t this_step_ms = step_time_ms(time_buffer_s, i);
	uint64_t next_step_ms = step_time_ms(time_buffer_s, i + 1);
	if(this_step_ms <= time_ms_mapped && time_ms_mapped < next_step_ms) {
	    float interpolation =
		(float)(time_ms_mapped - this_step_ms)/(float)(next_step_ms - this_step_ms);
	    return {i, i+1, interpolation};
	}
    }
    assert(0 && "invalid animation step");
    return {};
}

static int skin_joints_data_index_of_joint(SkinJointsData *skin_data, cgltf_node *joint) {
    for(uint64_t i = 0; i < skin_data->len; ++i) {
	if(skin_data->joints[i] == joint) {
	    return i;
	}
    }
    return JK_NODE_IS_NOT_SKIN_JOINT;
}

//##################################################
// @TAG jdk: main stuff


//##################################################
// @TAG jdk: model loading functions

static void gltf_load_node_meta(GLTFLoadParams *params, GLTFModel *model_result,
	cgltf_data *data, cgltf_node *node, Mat4 parent_world_matrix);

GLTFModel *gltf_load(GLTFLoadParams *params) {
    GLTFModel *result = zero_alloc<GLTFModel>();
    cgltf_data *data = NULL;
    Str8 file_dir = str8_dir_finish_with_slash(params->file_dir, &temp_allocator);
    Str8 path = str8_cat(file_dir, params->file_name, &temp_allocator);
    cgltf_result parse_result =
	cgltf_parse_file(&params->options, cstr_from_str8(path, &temp_allocator), &data);
    assert(parse_result == cgltf_result_success);
    cgltf_scene *scene = data->scene;
    for(uint64_t inode = 0; inode < scene->nodes_count; ++inode) {
	cgltf_node *node = scene->nodes[inode];
	gltf_load_node_meta(params, result, data, node, mat4(1.f));
    }
    result->data = data;
    return result;
}
// jdk: prepare gl buffers and attributes, static transforms (fallback...),
// skinning data, animation metadata (no actual loading of animations)
static void gltf_load_node_meta(GLTFLoadParams *params, GLTFModel *model_result,
	cgltf_data *data, cgltf_node *node, Mat4 parent_world_matrix) {
    // jdk: result
    NodeMeta *node_meta = list_push(&model_result->nodes_meta);
    // jdk: used to find the node_meta
    node_meta->key = node;
    // jdk: transform
    Vec3 scale = (node->has_scale) ? vec3_ptr(node->scale) : vec3(1.f);
    Quat rotation = (node->has_rotation) ? quat_ptr(node->rotation) : quat_identity();
    Vec3 translation = (node->has_translation) ? vec3_ptr(node->translation) : vec3(0.f);
    Transform transform = make_transform(translation, rotation, scale);
    Mat4 matrix = (node->has_matrix) ? mat4_copy_buffer(node->matrix) : make_mat4_transform(transform);
    Mat4 world_matrix = mat4_mul(parent_world_matrix, matrix);

    node_meta->static_matrix = matrix;
    node_meta->matrix = matrix;
    node_meta->world_matrix = world_matrix;

    // jdk: animations
    for(uint64_t ianim = 0; ianim < data->animations_count; ++ianim) {
	cgltf_animation *anim = &data->animations[ianim];
	AnimMeta *anim_meta = NULL;
	for(uint64_t ichannel = 0; ichannel < anim->channels_count; ++ichannel) {
	    cgltf_animation_channel *channel = &anim->channels[ichannel];
	    if(channel->target_node != node)
		continue;
	    if(!anim_meta) {
		anim_meta = link_push(&node_meta->anim_first, {str8(anim->name)});
	    }
	    cgltf_animation_sampler *sampler = channel->sampler;
	    // jdk: this is ok for animations because the spec says: byteStride only for vertex attribs
	    // => animation data shouldn't ever be interleaved
	    // TODO(jdk): add interleaving assertion
	    Str8 input_data =
		get_anim_sampler_accessor_data(model_result, sampler->input, params->bin_dir);
	    Str8 output_data =
		get_anim_sampler_accessor_data(model_result, sampler->output, params->bin_dir);
	    uint64_t num_frames = sampler->input->count;
	    link_push(&anim_meta->channel_first, {channel->target_path,
		    sampler->interpolation, num_frames, input_data, output_data});
	}
    }

    // jdk: mesh
    cgltf_mesh *mesh = node->mesh;
    cgltf_skin *skin = node->skin;
    if(mesh) {
	// jdk: skin
	if(skin) {
	    node_meta->has_skin = true;
	    node_meta->skin_data = {
		skin->joints,
		alloc<Mat4>(skin->joints_count),
		skin->joints_count
	    };
	    // TODO(jdk): can multiple sets of inverse bind matrices share the same buffer view??
	    cgltf_buffer_view *ibm_view = skin->inverse_bind_matrices->buffer_view;
	    Str8 ibm_data = get_view_bin_data(model_result, ibm_view, params->bin_dir);
	    node_meta->inverse_bind_matrices = {
		(Mat4 *)ibm_data.ptr,
		skin->inverse_bind_matrices->count
	    };
	}
	// TODO(jdk): morph targets?
	// jdk: setup primitives meta container
	node_meta->primitives_meta = make_stack<PrimMeta>(mesh->primitives_count);
	for(uint64_t iprim = 0; iprim < mesh->primitives_count; ++iprim) {
	    cgltf_primitive *prim = &mesh->primitives[iprim];

	    PrimMeta *prim_meta = stack_push(&node_meta->primitives_meta, {.key = prim});

	    // jdk: material
	    cgltf_material *material = prim->material;
	    if(material->has_pbr_metallic_roughness) {
		
	    }

	    // TODO(jdk): account for case 
	    assert(prim->indices && "primitive doesn't have indices => all vertices, not handled yet");


	    // jdk: vertex attributes setup
	    glGenVertexArrays(1, &prim_meta->vao);
	    glBindVertexArray(prim_meta->vao);

	    glGenBuffers(1, &prim_meta->ebo);
	    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim_meta->ebo);
	    cgltf_buffer_view *indices_view = prim->indices->buffer_view;
	    Str8 index_data = get_view_bin_data(model_result, indices_view, params->bin_dir);
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.len, index_data.ptr, GL_STATIC_DRAW);
	    prim_meta->indices_count = prim->indices->count;
	    prim_meta->indices_type = gl_enum_from_cgltf_component_type(prim->indices->component_type);

	    // jdk: group attributes of same buffer view because same glBufferSubData
	    Stack<AttribGroup> view_attrib_groups =
		make_stack<AttribGroup>(prim->attributes_count);
	    uint64_t full_buffer_size = 0;
	    for(uint64_t iattrib = 0; iattrib < prim->attributes_count; ++iattrib) {
		cgltf_buffer_view *current_view = prim->attributes[iattrib].data->buffer_view;
		static cgltf_buffer_view *last_view = NULL;
		static AttribGroup *attrib_group = NULL;
		if(current_view != last_view) {
		    uint64_t num_left_over = prim->attributes_count - iattrib;
		    attrib_group = stack_push<AttribGroup>(&view_attrib_groups,
			    make_stack<cgltf_attribute *>(num_left_over));
		    last_view = current_view;
		    full_buffer_size += current_view->size;
		}
		stack_push<cgltf_attribute *>(attrib_group, &prim->attributes[iattrib]);
	    }

	    glGenBuffers(1, &prim_meta->vbo);
	    glBindBuffer(GL_ARRAY_BUFFER, prim_meta->vbo);
	    // TODO(jdk): always static draw???
	    glBufferData(GL_ARRAY_BUFFER, full_buffer_size, NULL, GL_STATIC_DRAW);
	    uint64_t sub_data_offset = 0;
	    for(uint64_t igroup = 0; igroup < view_attrib_groups.len; ++igroup) {
		AttribGroup *group = &view_attrib_groups.buf[igroup];
		if(group->len <= 0)
		    break;
		cgltf_buffer_view *view = group->buf[0]->data->buffer_view;
		Str8 attrib_data = get_view_bin_data(model_result, view, params->bin_dir);
		glBufferSubData(GL_ARRAY_BUFFER, sub_data_offset, view->size,
			(const void *)attrib_data.ptr);
		for(uint64_t iattrib = 0; iattrib < group->len; ++iattrib) {
		    cgltf_attribute *attrib = group->buf[iattrib];
		    GLenum component_type =
			gl_enum_from_cgltf_component_type(attrib->data->component_type);
		    uint64_t component_count = cgltf_type_component_count(attrib->data->type);
		    int location = get_attrib_location(str8(attrib->name));
		    uint64_t offset = sub_data_offset + attrib->data->offset;
		    uint64_t stride = attrib->data->buffer_view->stride;
		    if(attrib->type == cgltf_attribute_type_joints) {
			glVertexAttribIPointer(location, component_count, component_type, stride,
				(const void *)offset);
		    } else {
			glVertexAttribPointer(location, component_count, component_type, GL_FALSE, stride,
				(const void *)offset);
		    }
		    glEnableVertexAttribArray(location);
		}
		sub_data_offset += view->size;
	    }
	    glBindVertexArray(0);

	    // jdk: morph targets setup
	    for(uint64_t itarget = 0; itarget < prim->targets_count; ++itarget) {
		cgltf_morph_target *target = &prim->targets[itarget];
		// jdk: mt for morph target
		for(uint64_t i_mt_attrib = 0; i_mt_attrib < target->attributes_count; ++i_mt_attrib) {
		    cgltf_attribute *mt_attrib = &target->attributes[i_mt_attrib];
		    cgltf_accessor *mt_attrib_accessor = mt_attrib->data;
		    cgltf_buffer_view *mt_attrib_view = mt_attrib_accessor->buffer_view;

		    uint64_t mt_attrib_element_size = mt_attrib_accessor->count *
			cgltf_component_type_size(mt_attrib_accessor->component_type) *
			cgltf_type_component_count(mt_attrib_accessor->type);
		    assert(mt_attrib_view->stride <= mt_attrib_element_size &&
			    "morph target attribs have stride greater than element size\n"
			    "=> interleaved, not supported");
		    Str8 mt_attrib_data =
			get_view_bin_data(model_result, mt_attrib_view, params->bin_dir);
		    uint64_t offset = mt_attrib_accessor->offset;
		    uint64_t index = get_morph_attrib_location(str8(mt_attrib->name));
		    uint64_t count = mt_attrib_accessor->count;
		    if(prim_meta->morph_attribute_textures[index] == 0) {
			glGenTextures(1, &prim_meta->morph_attribute_textures[index]);
			glBindTexture(GL_TEXTURE_2D, prim_meta->morph_attribute_textures[index]);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, count, prim->targets_count);
			glBindTexture(GL_TEXTURE_2D, 0);
		    }
		    glBindTexture(GL_TEXTURE_2D, prim_meta->morph_attribute_textures[index]);
		    glTexSubImage2D(GL_TEXTURE_2D, 0 , 0, itarget, count, 1,
			    cgltf_type_tex_format(mt_attrib_accessor->type),
			    gl_enum_from_cgltf_component_type(mt_attrib_accessor->component_type),
			    mt_attrib_data.ptr + offset);
		    glBindTexture(GL_TEXTURE_2D, 0);
		}

	    }
	}
    }
    for(uint64_t ichild = 0; ichild < node->children_count; ++ichild) {
	cgltf_node *child = node->children[ichild];
	gltf_load_node_meta(params, model_result, data, child, world_matrix);
    }
}


//##################################################
// @TAG jdk: apply animated node transforms to model

static void gltf_apply_node_animations(GLTFModel *model, cgltf_node *node, Mat4 parent_world_matrix);
static void gltf_apply_joints(GLTFModel *model, cgltf_node *node);

void gltf_animate(GLTFModel *model) {
    cgltf_scene *scene = model->data->scene;
    Slice<cgltf_node *> nodes = make_slice(scene->nodes, scene->nodes_count);
    JK_SliceForeach(&nodes, node, { gltf_apply_node_animations(model, *node, mat4(1.f)); });
    JK_SliceForeach(&nodes, node, { gltf_apply_joints(model, *node); });
}

static void gltf_apply_node_animations(GLTFModel *model, cgltf_node *node,
	Mat4 parent_world_matrix) {
    NodeMeta *node_meta = list_find(&model->nodes_meta, match_node, node);
    assert(node_meta);

    Mat4 matrix = {};
    AnimMeta *anim_meta = link_find(node_meta->anim_first, match_anim, model->anim.name_current);
    if(anim_meta) {
	Vec3 translation = {};
	Quat rotation = {};
	Vec3 scale = {};

	ChannelMeta *translation_meta = link_find(anim_meta->channel_first, match_channel,
		cgltf_animation_path_type_translation);
	if(translation_meta) {
	    AnimStep step = get_anim_step(translation_meta, model->anim.time_ms);
	    Vec3 *vec3_buffer = (Vec3 *)translation_meta->output.ptr;
	    Vec3 t0 = vec3_buffer[step.i0];
	    Vec3 t1 = vec3_buffer[step.i1];
	    // TODO(jdk): add step interpolation and that weird spline thingy...
	    Vec3 t_res = vec3_lerp(t0, t1, step.interpolation_factor);
	    translation = t_res;
	} else {
	    translation = vec3(node->translation);
	}

	ChannelMeta *rotation_meta = link_find(anim_meta->channel_first, match_channel,
		cgltf_animation_path_type_rotation);
	if(rotation_meta) {
	    // @TODO(jdk): swap Quat layout => w to back
	    AnimStep step = get_anim_step(rotation_meta, model->anim.time_ms);
	    Quat *quat_buffer = (Quat *)rotation_meta->output.ptr;
	    Quat q0 = quat_buffer[step.i0];
	    Quat q1 = quat_buffer[step.i1];
	    Quat q_res = slerp(q0, q1, step.interpolation_factor);
	    rotation = q_res;
	} else {
	    rotation = quat(node->rotation);
	}

	ChannelMeta *scale_meta = link_find(anim_meta->channel_first, match_channel,
		cgltf_animation_path_type_scale);
	if(scale_meta) {
	    AnimStep step = get_anim_step(scale_meta, model->anim.time_ms);
	    Vec3 *vec3_buffer = (Vec3 *)scale_meta->output.ptr;
	    Vec3 s0 = vec3_buffer[step.i0];
	    Vec3 s1 = vec3_buffer[step.i1];
	    Vec3 s_res = vec3_lerp(s0, s1, step.interpolation_factor);
	    scale = s_res;
	} else {
	    scale = vec3(node->scale);
	}

	ChannelMeta *weights_meta = link_find(anim_meta->channel_first, match_channel,
		cgltf_animation_path_type_weights);
	if(weights_meta) {
	    // // @TODO(jdk): handle component sizes
	    AnimStep step = get_anim_step(weights_meta, model->anim.time_ms);
	    uint64_t num_weights = weights_meta->output.len/weights_meta->input.len;
	    assert(num_weights <= 64);
	    float *fbuffer = (float *)weights_meta->output.ptr;
	    float *w0 = &fbuffer[step.i0 * num_weights];
	    float *w1 = &fbuffer[step.i1 * num_weights];
	    float *w_res = lerp(w0, w1, num_weights, step.interpolation_factor, &temp_allocator);
	    node_meta->morph_weights = make_slice<float>(w_res, num_weights);
	} else {
	    node_meta->morph_weights = make_slice<float>(NULL, 0);
	}

	matrix = make_mat4_transform(translation, rotation, scale);
    } else {
	matrix = node_meta->static_matrix;
    }

    Mat4 world_matrix = mat4_mul(parent_world_matrix, matrix);
    node_meta->matrix = matrix;
    node_meta->world_matrix = world_matrix;

    for(uint64_t ichild = 0; ichild < node->children_count; ++ichild) {
	cgltf_node *child = node->children[ichild];
	gltf_apply_node_animations(model, child, world_matrix);
    }
}


//##################################################
// @TAG jdk: skeleton space matrices from calculated joint transforms

static void gltf_apply_skeleton_joints(GLTFModel *model, cgltf_node *joint, NodeMeta *skin_meta,
	Mat4 parent_skeleton_matrix);

static void gltf_apply_joints(GLTFModel *model, cgltf_node *node) {
    cgltf_skin *skin = node->skin;
    if(skin) {
	NodeMeta *skin_meta = list_find(&model->nodes_meta, match_node, node);
	cgltf_node *skeleton = skin->skeleton;
	for(uint64_t ichild = 0; ichild < skeleton->children_count; ++ichild) {
	    gltf_apply_skeleton_joints(model, skeleton->children[ichild], skin_meta, mat4(1.f));
	}
    }
    for(uint64_t ichild = 0; ichild < node->children_count; ++ichild) {
	gltf_apply_joints(model, node->children[ichild]);
    }
}

static void gltf_apply_skeleton_joints(GLTFModel *model, cgltf_node *joint, NodeMeta *skin_meta,
	Mat4 parent_skeleton_matrix) {
    int index = skin_joints_data_index_of_joint(&skin_meta->skin_data, joint);
    if(index == JK_NODE_IS_NOT_SKIN_JOINT)
	return;
    NodeMeta *joint_meta = list_find(&model->nodes_meta, match_node, joint);
    assert(joint_meta);
    Mat4 skeleton_matrix = mat4_mul(parent_skeleton_matrix, joint_meta->matrix);
    skin_meta->skin_data.skeleton_matrices[index] = skeleton_matrix;
    for(uint64_t ichild = 0; ichild < joint->children_count; ++ichild) {
	gltf_apply_skeleton_joints(model, joint->children[ichild], skin_meta, skeleton_matrix);
    }
}



//#####################################################################################
// @TAG jdk: main gltf drawing funcs

static void gltf_draw_node(GLTFModel *model, Mat4 base_matrix, cgltf_node *node);

void gltf_draw(GLTFModel *model, Mat4 base_matrix) {
    cgltf_scene *scene = model->data->scene;
    for(uint64_t inode = 0; inode < scene->nodes_count; ++inode) {
	gltf_draw_node(model, base_matrix, scene->nodes[inode]);
    }
}

static void gltf_draw_node(GLTFModel *model, Mat4 base_matrix, cgltf_node *node) {
    cgltf_skin *skin = node->skin;
    cgltf_mesh *mesh = node->mesh;
    NodeMeta *node_meta = list_find(&model->nodes_meta, match_node, node);
    assert(node_meta);
    if(mesh) {
	// jdk: skin
	if(skin) {
	    for(uint64_t ijoint = 0; ijoint < node_meta->skin_data.len; ++ijoint) {
		Mat4 joint_skeleton_matrix = node_meta->skin_data.skeleton_matrices[ijoint];
		Mat4 bind_matrix = node_meta->inverse_bind_matrices.ptr[ijoint];
		// TODO(jdk): account for doubles
		Mat4 joint_matrix = mat4_mul(joint_skeleton_matrix, bind_matrix);
		uint32_t location_joint_matrix = main_shader.location_joint_matrices + ijoint;
		glUniformMatrix4fv(location_joint_matrix, 1, GL_FALSE, (float *)&joint_matrix);
	    }
	}
	Mat4 final_matrix = mat4_mul(base_matrix, node_meta->world_matrix);
	glUniformMatrix4fv(main_shader.location_world, 1, GL_FALSE, (float *)&final_matrix);
	glUniform1i(main_shader.location_has_skin, node_meta->has_skin);
	// jdk: morph targets
	uint32_t morph_location = glGetUniformLocation(main_shader.id, "morph_weights");
	glUniform1fv(morph_location, node_meta->morph_weights.len, node_meta->morph_weights.ptr);
	for(uint64_t iprim = 0; iprim < mesh->primitives_count; ++iprim) {
	    cgltf_primitive *prim = &mesh->primitives[iprim];
	    PrimMeta *info = stack_find(&node_meta->primitives_meta, match_prim, prim);

	    for(uint64_t imorph_attrib = 0; imorph_attrib < JK_NUM_MORPH_ATTRIBS; ++imorph_attrib) {
		if(info->morph_attribute_textures[imorph_attrib] != 0) {
		    Arena arena = make_arena(1024); // @TODO(jdk): get rid of this
		    Str8 index_str = str8_cfmt({"%d"}, imorph_attrib);
		    Str8 loc_str = str8_cat(str8("morph_texture"), index_str, &temp_allocator);
		    uint32_t location = glGetUniformLocation(main_shader.id,
			    cstr_from_str8(loc_str, &temp_allocator));

		    glUniform1i(location, imorph_attrib);
		    glActiveTexture(GL_TEXTURE0 + imorph_attrib);
		    glBindTexture(GL_TEXTURE_2D, info->morph_attribute_textures[imorph_attrib]);
		    destroy_arena(&arena);
		}
	    }

	    glBindVertexArray(info->vao);
	    glDrawElements(GL_TRIANGLES, info->indices_count, info->indices_type, NULL);
	    glBindVertexArray(0);
	}
    }
    // jdk: get child nodes call recursively
    for(uint64_t ichild = 0; ichild < node->children_count; ++ichild) {
	gltf_draw_node(model, base_matrix, node->children[ichild]);
    }
}
