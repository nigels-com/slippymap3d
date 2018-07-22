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

#include <iostream>

#include <unistd.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "tile.h"
#include "loader.h"
#include "input.h"
#include "global.h"


/**
 * @brief poll for events
 * @return false, if the program should end, otherwise true
 */
bool poll() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return false;
                }
                break;
            case SDL_KEYDOWN:
            {
                const uint64_t delta = (uint64_t(1)<<(64-player_state.zoom-5));
                switch (event.key.keysym.sym)
                {
                    case SDLK_c:     player_state.cross = !player_state.cross; break;
                    case SDLK_g:     player_state.grid = !player_state.grid; break;
                    case SDLK_i:     player_state.zoom = std::min(player_state.zoom+1, 19); break;
                    case SDLK_o:     player_state.zoom = std::max(player_state.zoom-1,  0); break;
                    case SDLK_LEFT:  player_state.x -= delta; break;
                    case SDLK_RIGHT: player_state.x += delta; break;
                    case SDLK_UP:    player_state.y += delta; break;
                    case SDLK_DOWN:  player_state.y -= delta; break;
                }
                break;
            }
                case SDL_MOUSEMOTION:
                handle_mouse_motion(event.motion);
                break;
            case SDL_MOUSEBUTTONDOWN:
                handle_mouse_button_down(event.button);
                break;
            case SDL_MOUSEBUTTONUP:
                handle_mouse_button_up(event.button);
                break;
            case SDL_MOUSEWHEEL:
                handle_mouse_wheel(event.wheel);
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                        window_state.width = event.window.data1;
                        window_state.height = event.window.data2;
                        glViewport(0, 0, window_state.width, window_state.height);
                        break;
                }
                break;
            default:
                break;
        }
    }
    return true;
}

void render(int zoom, uint64_t x, uint64_t y)
{
    Tile* center_tile = TileFactory::instance()->get_tile_at(zoom, x, y);

    // Clear with black
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.0,-1.0,1.0);
    glTranslatef(0.0,-1.0,0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-(window_state.width / 2), (window_state.width / 2), -(window_state.height / 2), (window_state.height / 2), -1000, 1000);


    // Rotate and and tilt the world geometry
    glRotated(viewport_state.angle_tilt, 1.0, 0.0, 0.0);
    glRotated(viewport_state.angle_rotate, 0.0, 0.0, -1.0);

    uint64_t m  = zoom ? uint64_t(1)<<(64-zoom) : ~uint64_t(0);
    uint64_t fx = zoom ? x%m : x;
    uint64_t fy = zoom ? y%m : y;

//    std::cout << center_tile->x << " " << center_tile->y << " " << m << " " << fx << " " << fy << std::endl;

    // Render the slippy map parts
    glEnable(GL_TEXTURE_2D);

        // Draw tiles
        glPushMatrix();
            glScalef(512.0, 512.0, 1.0);
            glTranslated(-double(fx)/m, -double(fy)/m, 0);

            static const int left = -2;
            static const int right = 2;
            static const int top = 2;
            static const int bottom = -2;

            // Start 'left' and 'bottom' tiles from the center tile and render up to 'top' and
            // 'right' tiles from the center tile
            Tile * current = center_tile->get(left, bottom);
            for (int y = bottom; y <= top; y++) 
            {
                for (int x = left; x <= right; x++) 
                {
                    if (current->valid())
                    {
                        Tile * tile = current;
                        float minUV[2] = { 0, 0 };
                        float maxUV[2] = { 1, 1 };

                        // Dummy means it doesn't exist or hasn't loaded yet
                        // If so, look for an ancestor
                        if (tile->texid == TileFactory::instance()->get_dummy())
                        {
                             while (tile && tile->texid==TileFactory::instance()->get_dummy())
                             {
                                tile = tile->get_parent(minUV, maxUV);
                                if (tile && tile->texid == 0)
                                {
                                    Loader::instance()->open_image(*tile);
                                }
                            }
//                          std::cout << minUV[0] << " " << minUV[1] << " " << maxUV[0] << " " << maxUV[1] << std::endl;
                        }
                        else 
                        {
                            if (tile && tile->texid == 0)
                            {
                                Loader::instance()->open_image(*tile);
                            }
                        }

                        if (tile && tile->texid!=TileFactory::instance()->get_dummy())
                        {
                            glBindTexture(GL_TEXTURE_2D, tile->texid);
                            glPushMatrix();
                                glTranslated(x, y, 0);
                                glBegin(GL_QUADS);
                                    glTexCoord2f(minUV[0], minUV[1]); glVertex2f(0.0, 0.0);
                                    glTexCoord2f(minUV[0], maxUV[1]); glVertex2f(0.0, 1.0);
                                    glTexCoord2f(maxUV[0], maxUV[1]); glVertex2f(1.0, 1.0);
                                    glTexCoord2f(maxUV[0], minUV[1]); glVertex2f(1.0, 0.0);
                                glEnd();
                            glPopMatrix();
                        }
                    }
                    current = current->get_west();
                }
                current = current->get(-(std::abs(left) + std::abs(right) + 1), 1);
            }
        glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    // Draw grid
    if (player_state.grid)
    {
        glColor3d(1.0, 1.0, 1.0);
        glPushMatrix();
            glScalef(512.0, 512.0, 1.0);
            glTranslated(-double(fx)/m, -double(fy)/m, 0);

            // Start 'left' and 'top' tiles from the center tile and render down to 'bottom' and
            // 'right' tiles from the center tile
            current = center_tile->get(left, bottom);
            for (int y = bottom; y <= top; y++) {
                for (int x = left; x <= right; x++) {

                    if (current->valid())
                    {
                        // Render the tile itself at the correct position
                        glPushMatrix();
                            glTranslated(x, y, 0);
                            glBegin(GL_LINE_LOOP);
                                glVertex2f(0.0, 0.0);
                                glVertex2f(0.0, 1.0);
                                glVertex2f(1.0, 1.0);
                                glVertex2f(1.0, 0.0);
                                glVertex2f(0.0, 0.0);
                            glEnd();
                        glPopMatrix();
                    }
                    current = current->get_west();
                }
                current = current->get(-(std::abs(left) + std::abs(right) + 1), 1);
            }
        glPopMatrix();
    }

    // Draw grid
    if (player_state.cross)
    {
        glColor3d(0.0, 0.0, 0.0);
        glLineWidth(3.0);
        glBegin(GL_LINES);
            glVertex2f(-6,  0);
            glVertex2f( 6,  0);
            glVertex2f( 0, -6);
            glVertex2f( 0,  6);
        glEnd();

        glColor3d(1.0, 1.0, 1.0);
        glLineWidth(1.0);
        glBegin(GL_LINES);
            glVertex2f(-5,  0);
            glVertex2f( 5,  0);
            glVertex2f( 0, -5);
            glVertex2f( 0,  5);
        glEnd();
#if 0
        // Draw the players avatar at the center of the screen
        glColor3d(1.0, 0.5, 0.0);
        glBegin(GL_TRIANGLES);
            glVertex3f(-10, -15, 1);
            glVertex3f( 10, -15, 1);
            glVertex3f(  0,  10, 1);
        glEnd();
        glColor3d(1.0, 1.0, 1.0);
#endif
    }
}

int main()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Could not initialize SDL video: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create an OpenGL window
    SDL_Window* window = SDL_CreateWindow("slippymap3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Could not create SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_GetWindowSize(window, &window_state.width, &window_state.height);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long base_time = spec.tv_sec * 1000 + round(spec.tv_nsec / 1.0e6);
    int frames = 0;
    while(true) {
        if (!poll()) {
            break;
        }
        frames++;

        clock_gettime(CLOCK_REALTIME, &spec);
        long time_in_mill = spec.tv_sec * 1000 + round(spec.tv_nsec / 1.0e6);
        if ((time_in_mill - base_time) > 1000.0) {
            std::cout << frames * 1000.0 / (time_in_mill - base_time) << " fps" << std::endl;
            base_time = time_in_mill;
            frames=0;
        }

        render(player_state.zoom, player_state.x, player_state.y);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

