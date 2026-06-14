#ifndef JK_GAME_H
#define JK_GAME_H

#include "camera.hpp"

struct Game {
    F64 delta_time;
    Camera camera = make_camera();
};

#endif
