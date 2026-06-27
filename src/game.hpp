#ifndef JK_GAME_H
#define JK_GAME_H

#include "jmath.hpp"
#include "camera.hpp"

struct Game {
    double delta_time;
    Camera camera = make_camera();
    Mat4 view_matrix = mat4(1.f);
    Mat4 projection_matrix = mat4(1.f);
};

#endif
