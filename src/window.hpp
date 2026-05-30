#ifndef WINDOW_H
#define WINDOW_H

// #include "camera.hpp"
#include "base/core.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

constexpr int window_width = 1280;
constexpr int window_height = 720;
/*
#define CAM_MOVE_SPEED 3.0f
#define CAM_ROTATE_SPEED 35.f
*/

GLFWwindow *window_setup();
// void handle_input(GLFWwindow *window, Camera *camera, float delta_time);

#endif
