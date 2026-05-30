// @TAG(jdk): the code
#include "base/core.hpp"
#include "base/mat.hpp"
#include "base/vec.hpp"
#include "base/quat.hpp"
#include "base/arena.hpp"
#include "base/str.hpp"

#include "window.hpp"
#include "shader.hpp"
#include "gltf_load.hpp"
#include "ui.hpp"

#include <stb_image.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "ext/cgltf.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define JK_DUMP_PERSISTENT_ALLOC 0
#define JK_PRINT_ALLOCATOR_STATE 0

#define JK_FILE_NAME "AnimatedMorphCube.gltf"
// #define JK_FILE_NAME "RiggedFigure.gltf"
#define JK_ANIM_NAME "Square"
// #define JK_ANIM_NAME "anim_0"

U64 game_time_ms_u64() {
    return (U64)(glfwGetTime() * 1000.0);
}

F64 game_time_ms_f64() {
    return glfwGetTime() * 1000.0;
}

// jdk: for random data that is typically erased at the end of some long procedure via savepoint or
// otherwise at end of frame
Arena main_frame_arena = {0};
// jdk: for random data that should not be erased ever
Arena main_persistent_arena = {0};
// jdk: for files, that are never freed
Arena main_file_arena = {0};
// TODO(jdk): maybe later introduce allocators per level...

const F32 window_color[] = {
    (F32)0x22/(F32)0xFF,
    (F32)0x22/(F32)0xFF,
    (F32)0x22/(F32)0xFF,
};

int main() {
    //##################################################
    // jdk: allocators
    main_frame_arena = make_arena_cleared(JK_KiB(256), '#');
    main_persistent_arena = make_arena_cleared(JK_KiB(256), '%');
    main_file_arena = make_arena(JK_MiB(128));

    Context default_context = {
	&main_frame_arena,
	&main_persistent_arena,
	&main_file_arena,
	&malloc,
	&free
    };

    //##################################################
    // jdk: window setup
    GLFWwindow *window = window_setup();

    // jdk: shaders
    U32 shader_program = create_shader_vf(
	    "./src/shaders/main_vs.glsl",
	    "./src/shaders/main_fs.glsl");
    U32 loc_world = glGetUniformLocation(shader_program, "world");
    U32 loc_view = glGetUniformLocation(shader_program, "view");
    U32 loc_proj = glGetUniformLocation(shader_program, "proj");
    U32 loc_joint_matrices = glGetUniformLocation(shader_program, "joint_matrices");
    U32 loc_has_skin = glGetUniformLocation(shader_program, "has_skin");

    Mat4 view = make_mat4_look_at(vec3(2,2,3), vec3(0,0,0), vec3(0,1,0));
    Mat4 projection = make_mat4_perspective(JK_Rad32(60.f), (F32)window_width/(F32)window_height, 0.1f, 100.f);

    TextObject *text1 = new_text_object(&default_context, str8c("GLTF animation test v0.1"));

    // jdk: model load
    GLTFLoadParams params = {
	str8c("./model/example_morph"), str8c(JK_FILE_NAME), {}, str8c("./model/example_morph")
	// str8c("./model/gltf"), str8c(JK_FILE_NAME), {}, str8c("./model/gltf")
    };
    GLTFModel *model = gltf_load(&default_context, &params);
    // TODO(jdk): anim duration into animation metadata??? => dunno for what reason

    const F32 font_scale = 2.f;

    F64 last_time = glfwGetTime();
    F64 delta_time = 0;
    F64 time_fps_counter = glfwGetTime();
    while(!glfwWindowShouldClose(window)) {
	// @TODO(jdk): test quat after switching layout
	Mat4 scale = make_mat4_scale(vec3(1.f));
	Quat q1 = quat_axis_angle(vec3(0.f, 1.f, 0.f), JK_Rad32(0.f));
	Quat q2 = quat_axis_angle(vec3(0.f, 1.f, 0.f), JK_Rad32(0.f));
	Quat q_rot = slerp(q1, q2, sinf(glfwGetTime()));
	Mat4 rotation = make_mat4_rotate_quat(q_rot);
	Mat4 translation = make_mat4_translate(vec3(0, 0 * sinf(glfwGetTime()), 0));
	Mat4 world = mat4_mul3(translation, rotation, scale);

	glClearColor(window_color[0], window_color[1], window_color[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_program);
	glUniformMatrix4fv(loc_view,  1, GL_FALSE, (float *)&view);
	glUniformMatrix4fv(loc_proj,  1, GL_FALSE, (float *)&projection);

	model->anim.time_ms = game_time_ms_u64();
	model->anim.name_current = str8c(JK_ANIM_NAME);
	gltf_animate(&default_context, model);

	model->draw.program = shader_program;
	model->draw.location_world_matrix = loc_world;
	model->draw.location_joint_matrices = loc_joint_matrices;
	model->draw.location_has_skin = loc_has_skin;
	model->draw.base_transform = world;
	gltf_draw(model);
	glUseProgram(0);


	//##################################################
	// jdk: text rendering
	draw_textbox_no_background(&default_context, text1, vec3_rgb8(180, 120, 80), font_scale,
				   20, 20, window_width, window_height);
	// jdk: fps counter...
	static TextObject *fps_textobj = NULL;
	if(!fps_textobj || glfwGetTime() - 1.0 > time_fps_counter) {
	    // TODO(jdk): make proper average, 1% low and stuff..
	    default_context.heap_free(fps_textobj);
	    I64 fps = (I64)(1.0/delta_time);
	    Str8 fps_counter_str = str8_cat(default_context.frame_arena,
					    str8_cstr("FPS: "),
					    str8_from_i64(&main_frame_arena, fps));
	    fps_textobj = new_text_object(&default_context, fps_counter_str);
	    time_fps_counter = glfwGetTime();
	}
	draw_textbox_no_background(&default_context, fps_textobj, vec3_rgb8(50, 100, 220), font_scale,
				   window_width - (fps_textobj->w * font_scale + 20), 20, window_width, window_height);

	//##################################################
	// jdk: end of frame
	glfwSwapBuffers(window);
	glfwPollEvents();
	// timer
	F64 current_time = glfwGetTime();
	delta_time = current_time - last_time;
	last_time = current_time;
	arena_reset(&main_frame_arena);
    }

#if JK_PRINT_ALLOCATOR_STATE
    printf("temp allocator offset: %lu\n", main_frame_arena.offset);
    printf("persist allocator offset: %lu\n", main_persistent_arena.offset);
    printf("file allocator offset: %lu\n", main_file_arena.offset);
#endif

#if JK_DUMP_PERSISTENT_ALLOC
    for(U64 i = 0; i < main_persistent_arena.offset; ++i) {
	putchar(((U8 *)main_persistent_arena.memory)[i]);
    }
    putchar('\n');
#endif
    exit(JK_SUCCESS);
}

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define CGLTF_IMPLEMENTATION
#include "ext/cgltf.h"

