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

#ifndef _SM3D_GLOBAL_H_
#define _SM3D_GLOBAL_H_

#include <cstdint>

/**
 * @brief the size of a tile in pixels
 */
#define TILE_SIZE (256.0)

/**
 * @brief holds the state of the window's width and height
 */
struct s_window_state {
    /**
     * @brief width of the window
     */
    int width;
    /**
     * @brief height of the window
     */
    int height;
};

/**
 * @brief hold the players current position
 */
struct s_player_state 
{
    bool grid = true;
    bool cross = true;

    // 17/121224/54208 @ level 64
    uint64_t x = uint64_t(121224)<<(64-17);
    uint64_t y = uint64_t(54208)<<(64-17);

    double zoom = 17;
};

extern struct s_window_state window_state;
extern struct s_player_state player_state;

#endif
