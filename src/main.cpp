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

#include <unistd.h>
#include <time.h>

#include <SDL2/SDL.h>

#include <GL/glew.h>

#include <curl/curl.h>

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "event.h"
#include "tile.h"
#include "tilefactory.h"
#include "loader.h"
#include "input.h"
#include "global.h"

#include <cmath>

// Compute the bottom left tile, and tile grid size
void visibleBounds(uint64_t width, uint64_t height, uint64_t bits, uint64_t z, uint64_t x, uint64_t y, uint64_t tile[2], uint64_t size[2])
{
   tile[0] = x - ((width >>1)<<(64-z-bits));
   tile[1] = y - ((height>>1)<<(64-z-bits));

   if (z>0)
   {
       tile[0] >>= (64-z);
       tile[1] >>= (64-z);
   }
   else
   {
       tile[0] = 0;
       tile[1] = 0;
   }

   const uint64_t tileSize = uint64_t(1)<<bits;
   size[0] = (width/tileSize) + 2;
   size[1] = (height/tileSize) + 2;
}

void drawTiles(Loader & loader, GLsizei width, GLsizei height, double zf, uint16_t z, uint64_t x, uint64_t y)
{
    const uint16_t bits = 9;
    const uint64_t tileSize = uint64_t(1)<<bits;
    const uint64_t levelSize = uint64_t(1)<<z;

    // View bounds in quad-tree co-ordinates, based on viewport center at x, y
    uint64_t tile[2];
    uint64_t size[2];
    visibleBounds(width/zf, height/zf, bits, z, x, y, tile, size);

    // Offset in pixels from bottom left to center
    const uint64_t fx = (x - (tile[0]<<(64-z))) >> (64-bits-z);
    const uint64_t fy = (y - (tile[1]<<(64-z))) >> (64-bits-z);

//  std::cout << bits << " " << tileSize << " " << " " << fx << " " << fy << std::endl;

    // Render the slippy map parts

    uint64_t j = 0;
    for (uint64_t y = tile[1]; j<=size[1]; ++y, ++j)
    {
        uint64_t i = 0;
        for (uint64_t x = tile[0]; i<=size[0]; ++x, ++i)
        {
//            std::cout << z << "/" << (x>>16) << "/" << (y>>16) << std::endl;
            Tile * current = TileFactory::instance()->get_tile(loader, z, x%levelSize, y%levelSize);
            if (current->valid())
            {
                float minUV[2] = { 0, 0 };
                float maxUV[2] = { 1, 1 };

                // Dummy means it doesn't exist or hasn't loaded yet
                // If so, look for an ancestor
                if (current->texid == TileFactory::instance()->get_dummy())
                {
                     while (current && current->texid==TileFactory::instance()->get_dummy())
                     {
                        current = current->get_parent(loader, minUV, maxUV);
                        if (current && current->texid == 0)
                        {
                            loader.open_image(*current);
                        }
                    }
                }
                else
                {
                    if (current && current->texid == 0)
                    {
                        loader.open_image(*current);
                    }
                }

                if (current && current->texid != TileFactory::instance()->get_dummy())
                {
                    glBindTexture(GL_TEXTURE_2D, current->texid);
                    glPushMatrix();
                        glScaled(zf, zf, 1);
                        glTranslated(-double(fx), -double(fy), 0);
                        glScaled(tileSize, tileSize, 1);
                        glTranslated(i, j, 0);
                        glBegin(GL_QUADS);
                            glTexCoord2f(minUV[0], minUV[1]); glVertex2f(0.0, 0.0);
                            glTexCoord2f(minUV[0], maxUV[1]); glVertex2f(0.0, 1.0);
                            glTexCoord2f(maxUV[0], maxUV[1]); glVertex2f(1.0, 1.0);
                            glTexCoord2f(maxUV[0], minUV[1]); glVertex2f(1.0, 0.0);
                        glEnd();
                    glPopMatrix();
                }
            }
        }
    }
}

Loader basemap(false, false, 19, "https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/", "", "./base/");
//Loader basemap(false, true, "https://tile.openstreetmap.org/", ".png", "./osm/");

void render(double zoom, uint64_t x, uint64_t y)
{
//    std::cout << zoom << std::endl;

    const uint16_t z = std::ceil(zoom);
    const double zf = std::pow(2.0, zoom-z);

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

    glPushMatrix();

        // Rotate and and tilt the world geometry
        glRotated(viewport_state.angle_tilt, 1.0, 0.0, 0.0);
        glRotated(viewport_state.angle_rotate, 0.0, 0.0, -1.0);

        // Draw tiles
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4d(1.0, 1.0, 1.0, 1.0);

            drawTiles(basemap, window_state.width, window_state.height, zf, z, x, y);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);

        // Draw grid
        if (player_state.grid)
        {
            // Translation as fraction of 512
            const uint64_t fx = (x<<z)>>(64-9);
            const uint64_t fy = (y<<z)>>(64-9);

            glColor3d(1.0, 1.0, 1.0);
            glPushMatrix();
                glScalef(zf, zf, 1.0);
                glTranslated(-double(fx), -double(fy), 0);
                glScalef(512.0, 512.0, 1.0);

                static const int left = -4;
                static const int right = 4;
                static const int top = 3;
                static const int bottom = -3;

                // Start 'left' and 'top' tiles from the center tile and render down to 'bottom' and
                // 'right' tiles from the center tile
                Tile * center = TileFactory::instance()->get_tile_at(basemap, z+1, x, y);
                Tile* current = center->get(basemap, left, bottom);
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
                        current = current->get_west(basemap);
                    }
                    current = current->get(basemap, -(std::abs(left) + std::abs(right) + 1), 1);
                }
            glPopMatrix();
        }

    glPopMatrix();

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
    }
}

int main()
{
    // Initialize CURL
    if (curl_global_init(CURL_GLOBAL_ALL) != 0)
    {
        std::cerr << "Could not initialize libcurl. " << std::endl;
        return 1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Could not initialize SDL video: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Swap on vsync
    SDL_GL_SetSwapInterval(1);

    // Create an OpenGL window
    window = SDL_CreateWindow("slippymap3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Could not create SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_GetWindowSize(window, &window_state.width, &window_state.height);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    clock_gettime(CLOCK_REALTIME, &timeKeyboardMouse);

    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long base_time = spec.tv_sec * 1000 + round(spec.tv_nsec / 1.0e6);
    int frames = 0;
    uint64_t d = 0;

    while (true)
    {
        if (!poll())
        {
            break;
        }

        clock_gettime(CLOCK_REALTIME, &spec);
        long now = spec.tv_sec * 1000 + round(spec.tv_nsec / 1.0e6);
        long idle = timeKeyboardMouse.tv_sec * 1000 + round(timeKeyboardMouse.tv_nsec / 1.0e6);

        // Hide mouse after 5s user idle
        SDL_ShowCursor((now - idle) < 5000.0);

        // Check for redisplay or new tiles downloaded
        if (redisplay || d!=downloaded || velocity.x || velocity.y)
        {
            d = downloaded;
            frames++;

            // Update position, if moving
            player_state.x += velocity.x;
            player_state.y += velocity.y;

            if ((now - base_time) > 1000) {
                std::cout << frames * 1000.0 / (now - base_time) << " fps" << std::endl;
                base_time = now;
                frames=0;
            }

            render(player_state.zoom, player_state.x, player_state.y);
            SDL_GL_SwapWindow(window);
            redisplay = false;
        }
        else
        {
            // 0.3% CPU usage on MacBook Pro
            boost::this_thread::sleep(boost::posix_time::milliseconds(50));
        }
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

