#ifndef WINDOW_H
#define WINDOW_H

// #include "camera.hpp"
#include "base/core.hpp"

#include "ext/glad/gl.h"
#include "ext/GLFW/glfw3.h"

#include <stdlib.h>
#include <stdio.h>

struct WindowInput {
    F32 axis_h;
    F32 axis_v;
    Vec2 wasd;
    // jdk: normalized
    Vec2 wasd_n;
    F32 axis_x;
    F32 axis_y;
    F32 axis_z;
    Vec3 sphere;

    // jdk: like in vim
    Vec2 hjkl;
    Vec2 hjkl_n;
};

constexpr int window_width = 1280;
constexpr int window_height = 720;

GLFWwindow *window_setup();
void input(GLFWwindow *window, WindowInput *input);

#endif
