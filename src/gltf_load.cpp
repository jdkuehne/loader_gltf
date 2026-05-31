#include "gltf_load.hpp"

// jdk: search conditions
static B8 match_prim(PrimMeta *x, cgltf_primitive *key) { return (x->key == key) ? JK_TRUE : JK_FALSE; }
static B8 match_node(NodeMeta *x, cgltf_node *key) { return (x->key == key) ? JK_TRUE : JK_FALSE; }
static B8 bin_data_file_has_path(GLTFBin *x, Str8 path) { return str8_equal(x->path, path); }
static B8 match_anim(AnimMeta *x, Str8 anim_name) { return str8_equal(x->name, anim_name); }
static B8 match_channel(ChannelMeta *x, cgltf_animation_path_type key) { return x->target == key; }

static U64 cgltf_component_type_size(cgltf_component_type x) {
    struct { cgltf_component_type type; U64 size; } static const map[] = {
	{cgltf_component_type_r_8, sizeof(I8)}, {cgltf_component_type_r_8u, sizeof(U8)},
	{cgltf_component_type_r_16, sizeof(I16)}, {cgltf_component_type_r_16u, sizeof(U16)},
	{cgltf_component_type_r_32u, sizeof(U32)}, {cgltf_component_type_r_32f, sizeof(F32)},
    };
    for(U64 i = 0; i < JK_ArrayLength(map); ++i) {
	if(map[i].type == x)
	    return map[i].size;
    }
    assert(0 && "invalid component type");
}

static GLenum gl_enum_from_cgltf_component_type(cgltf_component_type x) {
    struct { cgltf_component_type comp_type; GLenum gl_comp_type; } static const map[] = {
	{cgltf_component_type_r_8, GL_BYTE}, {cgltf_component_type_r_8u, GL_UNSIGNED_BYTE},
	{cgltf_component_type_r_16, GL_SHORT}, {cgltf_component_type_r_16u, GL_UNSIGNED_SHORT},
	{cgltf_component_type_r_32u, GL_UNSIGNED_INT}, {cgltf_component_type_r_32f, GL_FLOAT},
    };
    for(U64 i = 0; i < JK_ArrayLength(map); ++i) {
	if(map[i].comp_type == x) {
	    return map[i].gl_comp_type;
	}
    }
    assert(0 && "invalid component type");
}

// // @TODO(jdk): just use the one from the library
static U64 cgltf_type_component_count(cgltf_type x) {
    struct { cgltf_type type; U64 count; } static const map[] = {
	{cgltf_type_scalar, 1},
	{cgltf_type_vec2, 2}, {cgltf_type_vec3, 3}, {cgltf_type_vec4, 4},
	{cgltf_type_mat2, 4}, {cgltf_type_mat3, 9}, {cgltf_type_mat4, 16},
    };
    for(U64 i = 0; i < JK_ArrayLength(map); ++i) {
	if(map[i].type == x) {
	    return map[i].count;
	}
    }
    assert(0 && "cannot get component count of invalid type");
}

static GLenum cgltf_type_tex_format(cgltf_type x) {
    struct { cgltf_type type; GLenum format; } static const map[] = {
	{cgltf_type_scalar, GL_RED}, {cgltf_type_vec2, GL_RG},
	{cgltf_type_vec3, GL_RGB}, {cgltf_type_vec4, GL_RGBA},
    };
    for(U64 i = 0; i < JK_ArrayLength(map); ++i) {
	if(x == map[i].type) {
	    return map[i].format;
	}
    }
    assert(0 && "cannot get pixel format for cgltf_type");
}

static I32 get_attrib_location(Str8 attrib_name) {
    struct { const char *name; I32 location; } static const map[] = {
	{"POSITION", 0}, {"NORMAL", 1}, {"TANGENT", 2},
	{"TEXCOORD_0", 3}, {"TEXCOORD_1", 4}, {"COLOR_0", 5}, {"COLOR_1", 6},
	{"JOINTS_0", 7}, {"JOINTS_1", 8}, {"WEIGHTS_0", 9}, {"WEIGHTS_1", 10},
    };
    for(U64 i = 0; i < JK_ArrayLength(map); ++i) {
	if(str8_equal(attrib_name, str8c(map[i].name))) {
	    return map[i].location;
	}
    }
    exit(JK_ERROR_UNKNOWN_PRIMITIVE_ATTRIB);
}

static I32 get_morph_attrib_location(Str8 attrib_name) {
    struct { const char *name; I32 location; } static const map[] = {
	{"POSITION", 0}, {"NORMAL", 1}, {"TANGENT", 2},
	{"TEXCOORD_0", 3}, {"TEXCOORD_1", 4}, {"COLOR_0", 5}, {"COLOR_1", 6},
    };
    for(U64 i = 0; i < JK_ArrayLength(map); ++i) {
	if(str8_equal(attrib_name, str8c(map[i].name))) {
	    return map[i].location;
	}
    }
    exit(JK_ERROR_UNKNOWN_PRIMITIVE_ATTRIB);
}

// // @TODO(jdk): no handling for ./... vs ... paths
// jdk: maybe introduce reference counting for files?
static Str8 get_view_bin_data(Context *context, GLTFModel *model, cgltf_buffer_view *view, Str8 dir) {
    dir = str8_dir_finish_with_slash(context->frame_arena, dir);
    Str8 path = str8_cat(context->frame_arena, dir, str8_cstr(view->buffer->uri));
    GLTFBin *entry = list_find(&model->bin_files, &bin_data_file_has_path, path);
    if(entry) {
	return str8_substr(entry->data, view->offset, view->size);
    } else {
	Str8 buffer_data = file_read_full_to_str8(path);
	Str8 view_data = str8_substr(buffer_data, view->offset, view->size);
	GLTFBin new_entry = {str8_copy(context->persistent_arena, path), buffer_data};
	list_push(&model->bin_files, new_entry);
	return view_data;
    }
}

static Str8 get_anim_sampler_accessor_data(Context *context, GLTFModel *model,
	cgltf_accessor *accessor /*input or output*/, Str8 bin_dir) {
    cgltf_buffer_view *view = accessor->buffer_view;
    Str8 view_data = get_view_bin_data(context, model, view, bin_dir);
    U64 offset = accessor->offset;
    U64 component_count = cgltf_type_component_count(accessor->type);
    U64 component_size = cgltf_component_type_size(accessor->component_type);
    U64 size = accessor->count * component_count * component_size;
    return str8_substr(view_data, offset, size);
}

static inline U64 step_time_ms(F32 *buf_s, U64 i) { return (U64)(buf_s[i] * 1000.f); }
static AnimStep get_anim_step(ChannelMeta *channel_meta, U64 time_ms) {
    F32 *time_buffer_s = (F32 *)channel_meta->input.start;
    U64 num_frames = channel_meta->num_frames;
    U64 time_min_ms = step_time_ms(time_buffer_s, 0);
    U64 time_max_ms = step_time_ms(time_buffer_s, num_frames - 1);
    U64 time_ms_mapped = time_ms % (time_max_ms - time_min_ms) + time_min_ms;
    for(U64 i = 0; i < num_frames; ++i) {
	U64 this_step_ms = step_time_ms(time_buffer_s, i);
	U64 next_step_ms = step_time_ms(time_buffer_s, i + 1);
	if(this_step_ms <= time_ms_mapped && time_ms_mapped < next_step_ms) {
	    F32 interpolation =
		(F32)(time_ms_mapped - this_step_ms)/(F32)(next_step_ms - this_step_ms);
	    return {i, i+1, interpolation};
	}
    }
    assert(0 && "invalid animation step");
}

static I32 skin_joints_data_index_of_joint(SkinJointsData *skin_data, cgltf_node *joint) {
    for(U64 i = 0; i < skin_data->len; ++i) {
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

static void gltf_load_node_meta(Context *context, GLTFLoadParams *params, GLTFModel *model_result,
	cgltf_data *data, cgltf_node *node, Mat4 parent_world_matrix);

GLTFModel *gltf_load(Context *context, GLTFLoadParams *params) {
    GLTFModel *result = base::alloc<GLTFModel>();
    cgltf_data *data = NULL;
    Str8 file_dir = str8_dir_finish_with_slash(context->frame_arena, params->file_dir);
    Str8 path = str8_cat(context->frame_arena, file_dir, params->file_name);
    cgltf_result parse_result =
	cgltf_parse_file(&params->options, cstr_copy_from_str8(context->frame_arena, path), &data);
    assert(parse_result == cgltf_result_success);
    cgltf_scene *scene = data->scene;
    for(U64 inode = 0; inode < scene->nodes_count; ++inode) {
	cgltf_node *node = scene->nodes[inode];
	gltf_load_node_meta(context, params, result, data, node, mat4(1.f));
    }
    result->data = data;
    return result;
}
// jdk: prepare gl buffers and attributes, static transforms (fallback...),
// skinning data, animation metadata (no actual loading of animations)
static void gltf_load_node_meta(Context *context, GLTFLoadParams *params, GLTFModel *model_result,
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
    for(U64 ianim = 0; ianim < data->animations_count; ++ianim) {
	cgltf_animation *anim = &data->animations[ianim];
	AnimMeta *anim_meta = NULL;
	for(U64 ichannel = 0; ichannel < anim->channels_count; ++ichannel) {
	    cgltf_animation_channel *channel = &anim->channels[ichannel];
	    if(channel->target_node != node)
		continue;
	    if(!anim_meta) {
		anim_meta = link_push(context->persistent_arena, &node_meta->anim_first, {str8c(anim->name)});
	    }
	    cgltf_animation_sampler *sampler = channel->sampler;
	    // jdk: this is ok for animations because the spec says: byteStride only for vertex attribs
	    // => animation data shouldn't ever be interleaved
	    // TODO(jdk): add interleaving assertion
	    Str8 input_data =
		get_anim_sampler_accessor_data(context, model_result, sampler->input, params->bin_dir);
	    Str8 output_data =
		get_anim_sampler_accessor_data(context, model_result, sampler->output, params->bin_dir);
	    U64 num_frames = sampler->input->count;
	    link_push(context->persistent_arena, &anim_meta->channel_first, {channel->target_path,
		    sampler->interpolation, num_frames, input_data, output_data});
	}
    }

    // jdk: mesh
    cgltf_mesh *mesh = node->mesh;
    cgltf_skin *skin = node->skin;
    if(mesh) {
	// jdk: skin
	if(skin) {
	    node_meta->has_skin = JK_TRUE;
	    node_meta->skin_data = {
		skin->joints,
		base::alloc<Mat4>(skin->joints_count),
		skin->joints_count
	    };
	    // TODO(jdk): can multiple sets of inverse bind matrices share the same buffer view??
	    cgltf_buffer_view *ibm_view = skin->inverse_bind_matrices->buffer_view;
	    Str8 ibm_data = get_view_bin_data(context, model_result, ibm_view, params->bin_dir);
	    node_meta->inverse_bind_matrices = {
		(Mat4 *)ibm_data.start,
		skin->inverse_bind_matrices->count
	    };
	}
	// TODO(jdk): morph targets?
	// jdk: setup primitives meta container
	node_meta->primitives_meta = make_stack<PrimMeta>(context->persistent_arena, mesh->primitives_count);
	for(U64 iprim = 0; iprim < mesh->primitives_count; ++iprim) {
	    cgltf_primitive *prim = &mesh->primitives[iprim];

	    PrimMeta *prim_meta = stack_push(&node_meta->primitives_meta, {.key = prim});

	    // jdk: material
	    cgltf_material *material = prim->material;

	    // jdk: vertex attributes setup
	    glGenVertexArrays(1, &prim_meta->vao);
	    glBindVertexArray(prim_meta->vao);

	    glGenBuffers(1, &prim_meta->ebo);
	    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim_meta->ebo);
	    cgltf_buffer_view *indices_view = prim->indices->buffer_view;
	    Str8 index_data = get_view_bin_data(context, model_result, indices_view, params->bin_dir);
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.len, index_data.start, GL_STATIC_DRAW);
	    prim_meta->indices_count = prim->indices->count;
	    prim_meta->indices_type = gl_enum_from_cgltf_component_type(prim->indices->component_type);

	    // jdk: group attributes of same buffer view because same glBufferSubData
	    Stack<AttribGroup> view_attrib_groups =
		make_stack<AttribGroup>(context->frame_arena, prim->attributes_count);
	    U64 full_buffer_size = 0;
	    for(U64 iattrib = 0; iattrib < prim->attributes_count; ++iattrib) {
		cgltf_buffer_view *current_view = prim->attributes[iattrib].data->buffer_view;
		static cgltf_buffer_view *last_view = NULL;
		static AttribGroup *attrib_group = NULL;
		if(current_view != last_view) {
		    U64 num_left_over = prim->attributes_count - iattrib;
		    attrib_group = stack_push<AttribGroup>(&view_attrib_groups,
			    make_stack<cgltf_attribute *>(context->frame_arena, num_left_over));
		    last_view = current_view;
		    full_buffer_size += current_view->size;
		}
		stack_push<cgltf_attribute *>(attrib_group, &prim->attributes[iattrib]);
	    }

	    glGenBuffers(1, &prim_meta->vbo);
	    glBindBuffer(GL_ARRAY_BUFFER, prim_meta->vbo);
	    // TODO(jdk): always static draw???
	    glBufferData(GL_ARRAY_BUFFER, full_buffer_size, NULL, GL_STATIC_DRAW);
	    U64 sub_data_offset = 0;
	    for(U64 igroup = 0; igroup < view_attrib_groups.len; ++igroup) {
		AttribGroup *group = &view_attrib_groups.buf[igroup];
		if(group->len <= 0)
		    break;
		cgltf_buffer_view *view = group->buf[0]->data->buffer_view;
		Str8 attrib_data = get_view_bin_data(context, model_result, view, params->bin_dir);
		glBufferSubData(GL_ARRAY_BUFFER, sub_data_offset, view->size,
			(const void *)attrib_data.start);
		for(U64 iattrib = 0; iattrib < group->len; ++iattrib) {
		    cgltf_attribute *attrib = group->buf[iattrib];
		    GLenum component_type =
			gl_enum_from_cgltf_component_type(attrib->data->component_type);
		    U64 component_count = cgltf_type_component_count(attrib->data->type);
		    I32 location = get_attrib_location(str8c(attrib->name));
		    U64 offset = sub_data_offset + attrib->data->offset;
		    U64 stride = attrib->data->buffer_view->stride;
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
	    for(U64 itarget = 0; itarget < prim->targets_count; ++itarget) {
		cgltf_morph_target *target = &prim->targets[itarget];
		// jdk: mt for morph target
		for(U64 i_mt_attrib = 0; i_mt_attrib < target->attributes_count; ++i_mt_attrib) {
		    cgltf_attribute *mt_attrib = &target->attributes[i_mt_attrib];
		    cgltf_accessor *mt_attrib_accessor = mt_attrib->data;
		    cgltf_buffer_view *mt_attrib_view = mt_attrib_accessor->buffer_view;

		    U64 mt_attrib_element_size = mt_attrib_accessor->count *
			cgltf_component_type_size(mt_attrib_accessor->component_type) *
			cgltf_type_component_count(mt_attrib_accessor->type);
		    assert(mt_attrib_view->stride <= mt_attrib_element_size &&
			    "morph target attribs have stride greater than element size\n"
			    "=> interleaved, not supported");
		    Str8 mt_attrib_data =
			get_view_bin_data(context, model_result, mt_attrib_view, params->bin_dir);
		    U64 offset = mt_attrib_accessor->offset;
		    U64 index = get_morph_attrib_location(str8c(mt_attrib->name));
		    U64 count = mt_attrib_accessor->count;
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
			    mt_attrib_data.start + offset);
		    glBindTexture(GL_TEXTURE_2D, 0);
		}

	    }
	}
    }
    for(U64 ichild = 0; ichild < node->children_count; ++ichild) {
	cgltf_node *child = node->children[ichild];
	gltf_load_node_meta(context, params, model_result, data, child, world_matrix);
    }
}


//##################################################
// @TAG jdk: apply animated node transforms to model

static void gltf_apply_node_animations(Context *context, GLTFModel *model, cgltf_node *node, Mat4 parent_world_matrix);
static void gltf_apply_joints(GLTFModel *model, cgltf_node *node);

void gltf_animate(Context *context, GLTFModel *model) {
    cgltf_scene *scene = model->data->scene;
    Slice<cgltf_node *> nodes = make_slice(scene->nodes, scene->nodes_count);
    JK_SliceForeach(&nodes, node, { gltf_apply_node_animations(context, model, *node, mat4(1.f)); });
    JK_SliceForeach(&nodes, node, { gltf_apply_joints(model, *node); });
}

static void gltf_apply_node_animations(Context *context, GLTFModel *model, cgltf_node *node,
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
	    Vec3 *vec3_buffer = (Vec3 *)translation_meta->output.start;
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
	    Quat *quat_buffer = (Quat *)rotation_meta->output.start;
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
	    Vec3 *vec3_buffer = (Vec3 *)scale_meta->output.start;
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
	    U64 num_weights = weights_meta->output.len/weights_meta->input.len;
	    assert(num_weights <= 64);
	    F32 *fbuffer = (F32 *)weights_meta->output.start;
	    F32 *w0 = &fbuffer[step.i0 * num_weights];
	    F32 *w1 = &fbuffer[step.i1 * num_weights];
	    F32 *w_res = lerp(context->frame_arena, w0, w1, num_weights, step.interpolation_factor);
	    node_meta->morph_weights = make_slice<F32>(w_res, num_weights);
	} else {
	    node_meta->morph_weights = make_slice<F32>(NULL, 0);
	}

	matrix = make_mat4_transform(translation, rotation, scale);
    } else {
	matrix = node_meta->static_matrix;
    }

    Mat4 world_matrix = mat4_mul(parent_world_matrix, matrix);
    node_meta->matrix = matrix;
    node_meta->world_matrix = world_matrix;

    for(U64 ichild = 0; ichild < node->children_count; ++ichild) {
	cgltf_node *child = node->children[ichild];
	gltf_apply_node_animations(context, model, child, world_matrix);
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
	for(U64 ichild = 0; ichild < skeleton->children_count; ++ichild) {
	    gltf_apply_skeleton_joints(model, skeleton->children[ichild], skin_meta, mat4(1.f));
	}
    }
    for(U64 ichild = 0; ichild < node->children_count; ++ichild) {
	gltf_apply_joints(model, node->children[ichild]);
    }
}

static void gltf_apply_skeleton_joints(GLTFModel *model, cgltf_node *joint, NodeMeta *skin_meta,
	Mat4 parent_skeleton_matrix) {
    I32 index = skin_joints_data_index_of_joint(&skin_meta->skin_data, joint);
    if(index == JK_NODE_IS_NOT_SKIN_JOINT)
	return;
    NodeMeta *joint_meta = list_find(&model->nodes_meta, match_node, joint);
    assert(joint_meta);
    Mat4 skeleton_matrix = mat4_mul(parent_skeleton_matrix, joint_meta->matrix);
    skin_meta->skin_data.skeleton_matrices[index] = skeleton_matrix;
    for(U64 ichild = 0; ichild < joint->children_count; ++ichild) {
	gltf_apply_skeleton_joints(model, joint->children[ichild], skin_meta, skeleton_matrix);
    }
}



//#####################################################################################
// @TAG jdk: main gltf drawing funcs

static void gltf_draw_node(GLTFModel *model, cgltf_node *node);

void gltf_draw(GLTFModel *model) {
    cgltf_scene *scene = model->data->scene;
    for(U64 inode = 0; inode < scene->nodes_count; ++inode) {
	gltf_draw_node(model, scene->nodes[inode]);
    }
}

static void gltf_draw_node(GLTFModel *model, cgltf_node *node) {
    cgltf_skin *skin = node->skin;
    cgltf_mesh *mesh = node->mesh;
    NodeMeta *node_meta = list_find(&model->nodes_meta, match_node, node);
    assert(node_meta);
    if(mesh) {
	// jdk: skin
	if(skin) {
	    for(I64 ijoint = 0; ijoint < (I64)node_meta->skin_data.len; ++ijoint) {
		Mat4 joint_skeleton_matrix = node_meta->skin_data.skeleton_matrices[ijoint];
		Mat4 bind_matrix = node_meta->inverse_bind_matrices.ptr[ijoint];
		// TODO(jdk): account for doubles
		Mat4 joint_matrix = mat4_mul(joint_skeleton_matrix, bind_matrix);
		U32 location_joint_matrix = model->draw.location_joint_matrices + ijoint;
		glUniformMatrix4fv(location_joint_matrix, 1, GL_FALSE, (F32 *)&joint_matrix);
	    }
	}
	Mat4 final_matrix = mat4_mul(model->draw.base_transform, node_meta->world_matrix);
	glUniformMatrix4fv(model->draw.location_world_matrix, 1, GL_FALSE, (F32 *)&final_matrix);
	glUniform1i(model->draw.location_has_skin, node_meta->has_skin);
	// jdk: color
	glUniform3f(glGetUniformLocation(model->draw.program, "color"), 0.5, 0.3, 0.15);
	// jdk: morph targets
	glUniform1fv(glGetUniformLocation(model->draw.program, "morph_weights"),
		node_meta->morph_weights.len, node_meta->morph_weights.ptr);
	for(U64 iprim = 0; iprim < mesh->primitives_count; ++iprim) {
	    cgltf_primitive *prim = &mesh->primitives[iprim];
	    PrimMeta *info = stack_find(&node_meta->primitives_meta, match_prim, prim);

	    for(U64 imorph_attrib = 0; imorph_attrib < JK_NUM_MORPH_ATTRIBS; ++imorph_attrib) {
		if(info->morph_attribute_textures[imorph_attrib] != 0) {
		    Arena arena = make_arena(1024); // @TODO(jdk): get rid of this
		    Str8 index_str = str8_from_i64(&arena, imorph_attrib);
		    Str8 loc_str = str8_cat(&arena, str8c("morph_texture"), index_str);
		    U32 location = glGetUniformLocation(model->draw.program,
			    cstr_copy_from_str8(&arena, loc_str));

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
    for(U64 ichild = 0; ichild < node->children_count; ++ichild) {
	gltf_draw_node(model, node->children[ichild]);
    }
}
