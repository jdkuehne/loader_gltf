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

/*
void handle_input(GLFWwindow *window, Camera *camera, float delta_time) {
    float move_len = delta_time * CAM_MOVE_SPEED;
    float cam_angle_change = delta_time * CAM_ROTATE_SPEED;

    //movement
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
	camera->pos += camera->front * move_len;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
	camera->pos -= glm::normalize(glm::cross(camera->front, camera->up)) * move_len;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
	camera->pos -= camera->front * move_len;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
	camera->pos += glm::normalize(glm::cross(camera->front, camera->up)) * move_len;
    }
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
	camera->pos.y += move_len;
    }
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
	camera->pos.y -= move_len;
    }

    //camera angle
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
	pan_camera_deg(camera, cam_angle_change);
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
	pan_camera_deg(camera, -cam_angle_change);
    }
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
	tilt_camera_deg(camera, cam_angle_change);
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
	tilt_camera_deg(camera, -cam_angle_change);
    }
}
*/
