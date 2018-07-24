/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Christoph Brill
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <algorithm>
#include <iostream>
#include <cmath>

#include "input.h"
#include "global.h"
#include "tile.h"

struct s_input_state {
    bool left_mouse_down = false;
    bool right_mouse_down = false;
    bool middle_mouse_down = false;
} input_state;

struct s_viewport_state viewport_state;

double start_angle_rotate = 0.0;
double start_angle2 = 0.0;

double angle(int p1x, int p1y, int p2x, int p2y) {
    float xDiff = p2x - p1x;
    float yDiff = p2y - p1y;
    return -atan2(yDiff, xDiff) * (180 / M_PI);
}

void handle_mouse_motion(SDL_MouseMotionEvent &motion) {
#if 0
    if (input_state.left_mouse_down) {
        viewport_state.angle_rotate = angle(motion.x, motion.y, (window_state.width / 2), (window_state.height / 2)) - start_angle_rotate;
    }
#endif
    if (input_state.right_mouse_down) {
        if (motion.yrel<0)
        {
            player_state.zoom += motion.yrel/-50.0;
        }
        else
        {
            player_state.zoom -= motion.yrel/50.0;            
        }
        player_state.zoom = std::max<double>(player_state.zoom, 0);
        player_state.zoom = std::min<double>(player_state.zoom, 19);
    }
    if (input_state.left_mouse_down) {
        const uint64_t delta = uint64_t(1)<<int(std::floor((64-player_state.zoom-9)));
        const uint16_t z = std::ceil(player_state.zoom);
        const double zf = std::pow(2, player_state.zoom-z);
//        std::cout << zf << std::endl;
        player_state.x -= double(motion.xrel) * delta / zf;
        player_state.y += double(motion.yrel) * delta / zf;
//        std::cout << zf << " " << player_state.x << " " << player_state.y << std::endl;
    }
}

void handle_mouse_button_down(SDL_MouseButtonEvent &button) {
    if (button.button == 1) {
        input_state.left_mouse_down = true;
//        start_angle_rotate = angle(button.x, button.y, (window_state.width / 2), (window_state.height / 2)) - viewport_state.angle_rotate;
    } else if (button.button == 3) {
        input_state.right_mouse_down = true;
//        start_angle2 = angle(button.x, button.y, (window_state.width / 2), (window_state.height / 2)) - viewport_state.angle_tilt;
    } else if (button.button == 2) {
        input_state.middle_mouse_down = true;
    }
}

void handle_mouse_button_up(SDL_MouseButtonEvent &button) {
    if (button.button == 1) {
        input_state.left_mouse_down = false;
    } else if (button.button == 3) {
        input_state.right_mouse_down = false;
    } else if (button.button == 2) {
        input_state.middle_mouse_down = false;
    }
}

void handle_mouse_wheel(SDL_MouseWheelEvent &wheel) {
    player_state.zoom += wheel.y;
    player_state.zoom = std::max<double>(player_state.zoom, 0);
    player_state.zoom = std::min<double>(player_state.zoom, 19);
}
