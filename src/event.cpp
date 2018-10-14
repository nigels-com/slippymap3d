/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Christoph Brill
 * Copyright (c) 2018 Nigel Stewart (nigels@nigels.com)
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

#include <iostream>

#include <ctime>
#include <cmath>

#include <SDL2/SDL.h>

#include <GL/glew.h>

#include "global.h"
#include "input.h"

/**
 * @brief poll for events
 * @return false, if the program should end, otherwise true
 */
bool poll()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
            case SDL_KEYUP:
                redisplay = true;
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return false;
                }
                break;
            case SDL_KEYDOWN:
            {
                redisplay = true;
                const uint64_t delta = uint64_t(1)<<int(std::floor(64-player_state.zoom-5));
                switch (event.key.keysym.sym)
                {
                    case SDLK_c:     player_state.cross = !player_state.cross; break;
                    case SDLK_g:     player_state.grid = !player_state.grid; break;
                    case SDLK_i:     player_state.zoom = std::min<double>(player_state.zoom+1, 19); break;
                    case SDLK_o:     player_state.zoom = std::max<double>(player_state.zoom-1,  0); break;
                    case SDLK_LEFT:  player_state.x -= delta; break;
                    case SDLK_RIGHT: player_state.x += delta; break;
                    case SDLK_UP:    player_state.y += delta; break;
                    case SDLK_DOWN:  player_state.y -= delta; break;
                    case SDLK_TAB:   SDL_SetWindowFullscreen(window, (fullscreen = !fullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0); break;

                    case SDLK_a:     velocity.x -= delta/8;  break;
                    case SDLK_d:     velocity.x += delta/8;  break;
                    case SDLK_w:     velocity.y += delta/8;  break;
                    case SDLK_s:     velocity.y -= delta/8;  break;
                    case SDLK_SPACE: velocity.x = 0; velocity.y = 0; break;
                }
                break;
            }
            case SDL_MOUSEMOTION:
                redisplay = true;
                handle_mouse_motion(event.motion);
                break;
            case SDL_MOUSEBUTTONDOWN:
                redisplay = true;
                handle_mouse_button_down(event.button);
                break;
            case SDL_MOUSEBUTTONUP:
                redisplay = true;
                handle_mouse_button_up(event.button);
                break;
            case SDL_MOUSEWHEEL:
                redisplay = true;
                handle_mouse_wheel(event.wheel);
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                        redisplay = true;
                        window_state.width = event.window.data1;
                        window_state.height = event.window.data2;
                        glViewport(0, 0, window_state.width, window_state.height);
                        break;
                }
                break;
            default:
                break;
        }

        switch (event.type) {
            case SDL_KEYUP:
            case SDL_KEYDOWN:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
                clock_gettime(CLOCK_REALTIME, &timeKeyboardMouse);
                break;
            default:
                break;
        }
    }
    return true;
}