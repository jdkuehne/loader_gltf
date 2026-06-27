#ifndef WINDOW_H
#define WINDOW_H

#include "jmath.hpp"
#include "ext/glad/gl.h"
#include "ext/GLFW/glfw3.h"

#include <stdlib.h>
#include <stdio.h>

struct Input {
    float axis_h;
    float axis_v;
    Vec2 wasd;
    // jdk: normalized
    Vec2 wasd_n;
    float axis_x;
    float axis_y;
    float axis_z;
    Vec3 sphere;

    // jdk: like in vim
    Vec2 hjkl;
    Vec2 hjkl_n;
};

constexpr int window_width = 1280;
constexpr int window_height = 720;

inline bool key_pressed(GLFWwindow *window, int key) {
    return (glfwGetKey(window, key) == GLFW_PRESS);
}

GLFWwindow *window_setup();
void process_window_input(GLFWwindow *window, Input *input);

#endif
