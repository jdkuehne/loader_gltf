#include "window.hpp"

//glfw callbacks
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if(action == GLFW_PRESS) {
	switch(key) {
	    case GLFW_KEY_ESCAPE: {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	    }
	}
    }
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

static void APIENTRY gl_debug_message_callback(GLenum source,
	GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar *message,
	const void *userParam) {
    switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
	    printf("GL SEVERE: %s\n", message);
	    break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
	    printf("GL MEDIUM: %s\n", message);
	    break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
	    // TODO(jdk): replace these calls!
	    printf("GL LOW: %s\n", message);
	    break;
	}
	default: {
	    //do nothing
	}
    }
}

GLFWwindow *window_setup() {
    glfwInit();
    atexit(glfwTerminate);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "Dongus", NULL, NULL);
    if(!window) exit(JK_ERROR_WINDOWCREATE);

    glfwMakeContextCurrent(window);    
    int version = gladLoadGL(glfwGetProcAddress);
    if(!version) exit(JK_ERROR_LOADGLPROCS);

    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    //OpenGL features
    glEnable(GL_DEPTH_TEST);

    // glEnable(GL_BLEND);
    // glEnable(GL_FRAMEBUFFER_SRGB);
    // glEnable(GL_CULL_FACE);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glfw Callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glDebugMessageCallback(gl_debug_message_callback, NULL);
    return window;
}

void process_window_input(GLFWwindow *window, Input *input) {
    B8 w = key_pressed(window, GLFW_KEY_W);
    B8 a = key_pressed(window, GLFW_KEY_A);
    B8 s = key_pressed(window, GLFW_KEY_S);
    B8 d = key_pressed(window, GLFW_KEY_D);
    input->axis_h = (F32)(d - a);
    input->axis_v = (F32)(w - s);
    input->wasd = vec2(input->axis_h, input->axis_v);
    input->wasd_n = vec2_normalize(input->wasd);
    B8 e = key_pressed(window, GLFW_KEY_E);
    B8 q = key_pressed(window, GLFW_KEY_Q);
    input->axis_x = input->axis_h;
    input->axis_y = (F32)(e - q);
    input->axis_z = input->axis_v;
    input->sphere = vec3_normalize(vec3(input->axis_x, input->axis_y, input->axis_z));

    B8 h = key_pressed(window, GLFW_KEY_H);
    B8 j = key_pressed(window, GLFW_KEY_J);
    B8 k = key_pressed(window, GLFW_KEY_K);
    B8 l = key_pressed(window, GLFW_KEY_L);
    input->hjkl = vec2((F32)(l - h), (F32)(k - j));
    input->hjkl_n = vec2_normalize(input->hjkl);

    /*
    if(key_pressed(window, GLFW_KEY_K))
	camera_add_pitch(&camera, delta_time);
    if(key_pressed(window, GLFW_KEY_J))
	camera_add_pitch(&camera, -delta_time);
    if(key_pressed(window, GLFW_KEY_H))
	camera_add_yaw(&camera, -delta_time);
    if(key_pressed(window, GLFW_KEY_L))
	camera_add_yaw(&camera, delta_time);
    */
}
