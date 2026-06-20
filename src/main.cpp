// @TAG(jdk): the code
#include "base/core.hpp"
#include "base/mat.hpp"
#include "base/vec.hpp"
#include "base/quat.hpp"
#include "base/str.hpp"
#include "base/mem/allocator.hpp"

#include "shader.hpp"
#include "gltf_load.hpp"
#include "ui.hpp"
#include "game.hpp"

#include "ext/stb_image.h"
#include "ext/glad/gl.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

// @NOTE(jdk): apparently the fileapi header included for file.hpp defined APIENTRY and
// the winapi stuff in glfw does so as well (redefinition), but with include guards,
// so the one with include guards has to be at the end
#include "window.hpp"

#define JK_DUMP_PERSISTENT_ALLOC 0
#define JK_PRINT_ALLOCATOR_STATE 0

#define JK_FILE_NAME "MaterialTest.gltf"
#define JK_ANIM_NAME "anim_0"


inline U64 game_time_ms_u64() {
    return (U64)(glfwGetTime() * 1000.0);
}

inline F64 game_time_ms_f64() {
    return glfwGetTime() * 1000.0;
}

inline void set_clear_color_rgb8(U8 r, U8 g, U8 b) {
    Vec3 rgbf = vec3_rgb8(r, g, b);
    glClearColor(JK_CompPasteVec3(rgbf), 1.f);
}

void draw_fps_counter() {
    constexpr Vec3 color = vec3_rgb8(50, 100, 220);
    constexpr F32 period = 1.f;
    static F32 last_time = 0.f;
    static TextObject *text_obj = NULL;
    if(!text_obj || glfwGetTime() - period > last_time) {
	// TODO(jdk): make proper average, 1% low and stuff..
	delete_text_object(text_obj);
	I64 fps = (I64)(1.0/game.delta_time);
	Str8 fps_str = str8_from_i64(fps, &default_temp_allocator);
	Str8 text = str8_cat(str8c("FPS: "),
		str8_from_i64(fps, &default_temp_allocator), &default_temp_allocator);
	fps_textobj = new_text_object(fps_counter_str);
	time_fps_counter = glfwGetTime();
    }
    draw_textbox_no_background(fps_textobj, , font_scale,
			       window_width - (fps_textobj->w * font_scale + 20), 20, window_width, window_height);
}

int main() {
    Game game = {};
    game.camera.pos = vec3(0.f, 0.f, 2.f);
    GLFWwindow *window = window_setup();
    F32 aspect_ratio = (F32)window_width/(F32)window_height;
    Input input = {};

    setup_main_shader();
    main_shader_set_projection(JK_Rad32(60.f), aspect_ratio, 0.1f, 100.f);

    TextObject *text1 = new_text_object(str8c("GLTF animation test v0.1"));

    // jdk: model load
    Str8 model_dir = str8c("./model/gltf");
    GLTFLoadParams params = { model_dir, str8c(JK_FILE_NAME), {}, model_dir };
    GLTFModel *model = gltf_load(&params);

    const F32 font_scale = 2.f;

    F64 last_time = glfwGetTime();
    F64 time_fps_counter = glfwGetTime();
    while(!glfwWindowShouldClose(window)) {
	set_clear_color_rgb8(0x22, 0x22, 0x22);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// jdk: input, camera, shader update
	process_window_input(window, &input);
	camera_fps_move(&game.camera, vec3_scale(input.sphere, game.delta_time));
	camera_fps_turn(&game.camera, vec2_scale(input.hjkl_n, game.delta_time));
	main_shader_set_view_and_camera(&game.camera);

	// @TODO(jdk): test quat after switching layout

	glUseProgram(main_shader.id);
	model->anim.time_ms = game_time_ms_u64();
	model->anim.name_current = str8c(JK_ANIM_NAME);
	gltf_animate(model);
	gltf_draw(model, mat4(1.f));
	glUseProgram(0);


	//##################################################
	// jdk: text rendering
	draw_textbox_no_background(text1, vec3_rgb8(180, 120, 80), font_scale,
				   20, 20, window_width, window_height);
	// jdk: fps counter...

	//##################################################
	// jdk: end of frame
	glfwSwapBuffers(window);
	glfwPollEvents();
	// timer
	F64 current_time = glfwGetTime();
	game.delta_time = current_time - last_time;
	last_time = current_time;
	arena_reset(&default_temp_allocator_arena);
    }

    exit(JK_SUCCESS);
}

#define GLAD_GL_IMPLEMENTATION
#include "ext/glad/gl.h"
#define CGLTF_IMPLEMENTATION
#include "ext/cgltf.h"

