// @TAG(jdk): the code
#include "jbase.hpp"
#include "jmath.hpp"

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

#define JM_FONT_SCALE 2.f

inline uint64_t game_time_ms_u64() {
    return (uint64_t)(glfwGetTime() * 1000.0);
}

inline double game_time_ms_f64() {
    return glfwGetTime() * 1000.0;
}

inline void set_clear_color_rgb8(uint8_t r, uint8_t g, uint8_t b) {
    Vec3 rgbf = vec3_rgb8(r, g, b);
    glClearColor(JK_CompPasteVec3(rgbf), 1.f);
}

void draw_fps_counter(double delta_time) {
    Vec3 color = vec3_rgb8(50, 100, 220);
    constexpr float period = 1.f;
    static float last_time = 0.f;
    static TextObject *text_obj = NULL;
    if(!text_obj || glfwGetTime() - period > last_time) {
	// TODO(jdk): make proper average, 1% low and stuff..
	delete_text_object(text_obj);
	int fps = (int)(1.0/delta_time);
	Str8 text = str8_cfmt({"FPS: %d", &temp_allocator}, fps);
	text_obj = new_text_object(text);
	last_time = glfwGetTime();
    }
    draw_textbox_no_background(text_obj, color, JM_FONT_SCALE,
			       window_width - (text_obj->w * JM_FONT_SCALE + 20), 20, window_width, window_height);
}

int main() {
    Game game = {};
    game.camera.pos = vec3(0.f, 0.f, 2.f);
    GLFWwindow *window = window_setup();
    float aspect_ratio = (float)window_width/(float)window_height;
    Input input = {};

    setup_main_shader();
    uint32_t dumb_loc = glGetUniformLocation(main_shader.id, "world");
    main_shader_set_projection(jm_rad32(60.f), aspect_ratio, 0.1f, 100.f);

    TextObject *text1 = new_text_object(str8("GLTF animation test v0.1"));

    // jdk: model load
    Str8 model_dir = str8("./model/gltf");
    GLTFLoadParams params = { model_dir, str8(JK_FILE_NAME), {}, model_dir };
    GLTFModel *model = gltf_load(&params);


    double last_time = glfwGetTime();
    double time_fps_counter = glfwGetTime();
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
	model->anim.name_current = str8(JK_ANIM_NAME);
	gltf_animate(model);
	gltf_draw(model, mat4(1.f));
	glUseProgram(0);


	//##################################################
	// jdk: text rendering
	draw_textbox_no_background(text1, vec3_rgb8(180, 120, 80), JM_FONT_SCALE,
				   20, 20, window_width, window_height);
	// jdk: fps counter...
	draw_fps_counter(game.delta_time);

	//##################################################
	// jdk: end of frame
	glfwSwapBuffers(window);
	glfwPollEvents();
	// timer
	double current_time = glfwGetTime();
	game.delta_time = current_time - last_time;
	last_time = current_time;
	arena_reset(&temp_allocator_arena);
    }

    exit(JM_SUCCESS);
}
#define JBASE_IMPLEMENTATION
#include "jbase.hpp"

#define GLAD_GL_IMPLEMENTATION
#include "ext/glad/gl.h"
#define CGLTF_IMPLEMENTATION
#include "ext/cgltf.h"
#define JMATH_IMPLEMENTATION
#include "jmath.hpp"
