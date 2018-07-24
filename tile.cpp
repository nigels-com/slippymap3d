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

#include <sstream>
#include <cmath>

#include "tile.h"
#include "loader.h"

Tile::Tile(uint16_t zoom, uint64_t x, uint64_t y, GLuint texid) : 
    zoom(zoom), x(x), y(y), texid(texid)
{
}

Tile * Tile::get(int x_diff, int y_diff)
{
    return TileFactory::instance()->get_tile(zoom, x+x_diff, y+y_diff);
}

Tile * Tile::get_east() {
    return get(-1, 0);
}

Tile * Tile::get_north() {
    return get(0, -1);
}

Tile * Tile::get_south() {
    return get(0, 1);
}

Tile * Tile::get_west() {
    return get(1, 0);
}

Tile * Tile::get_parent()
{
    if (zoom>0)
    {
        return TileFactory::instance()->get_tile(zoom-1, x>>1, y>>1);
    }
    return NULL;
}

Tile * Tile::get_parent(float minUV[2], float maxUV[2]) const
{
    if (zoom>0)
    {
        minUV[0] *= 0.5;
        maxUV[0] *= 0.5;
        minUV[0] += (x&1) ? 0.5 : 0.0; 
        maxUV[0] += (x&1) ? 0.5 : 0.0;

        minUV[1] *= 0.5;
        maxUV[1] *= 0.5;
        minUV[1] += (y&1) ? 0.5 : 0.0; 
        maxUV[1] += (y&1) ? 0.5 : 0.0; 

        return TileFactory::instance()->get_tile(zoom-1, x>>1, y>>1);
    }

    return NULL;
}

std::string Tile::get_filename(bool tms, const std::string & ext)
{
    const uint64_t yy = tms ? y : (uint64_t(1)<<zoom) - 1 - y;
    std::stringstream filename;
    filename << this->zoom << "/" << this->x << '/' << yy << ext;
    return filename.str();
}

TileFactory* TileFactory::_instance = nullptr;

TileFactory::~TileFactory() {
    for (std::pair<std::string, Tile*> tile : tiles) {
        delete tile.second;
    }
    tiles.clear();
}

#if 0
int long2tilex(double lon, int z) {
    return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
}

int lat2tiley(double lat, int z) {
    return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
}

double tilex2long(int x, int z) {
    return x / pow(2.0, z) * 360.0 - 180;
}

double tiley2lat(int y, int z) {
    double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
    return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

double lonsize(int z) {
    return 1 / pow(2.0, z) * 180.0;
}

double latsize(double lat, int z) {
    // TODO this needs optimization but seems to work for now
    int tile = lat2tiley(lat, z);
    return (tiley2lat(tile, z) - tiley2lat(tile + 1, z)) / 2;
}
#endif

#if 0
Tile* TileFactory::get_tile(int zoom, double latitude, double longitude) {
    int x = long2tilex(longitude, zoom);
    int y = lat2tiley(latitude, zoom);
    return get_tile(zoom, x, y);
}
#endif

Tile* TileFactory::get_tile(uint16_t zoom, uint64_t x, uint64_t y) {
    std::string id = tile_id(zoom, x, y);
    std::map<std::string, Tile*>::iterator tile_iter = tiles.find(id);
    if (tile_iter != tiles.end()) {
        return tile_iter->second;
    }
    Tile* tile = new Tile(zoom, x, y, dummy);
    Loader::instance()->load_image(*tile);
    tiles[id] = tile;
    return tile;
}

Tile* TileFactory::get_tile_at(uint16_t zoom, uint64_t x, uint64_t y)
{
    x >>= (64-zoom);
    y >>= (64-zoom);
    std::string id = tile_id(zoom, int(x), int(y));
    std::map<std::string, Tile*>::iterator tile_iter = tiles.find(id);
    if (tile_iter != tiles.end()) {
        return tile_iter->second;
    }
    Tile* tile = new Tile(zoom, x, y, dummy);
    Loader::instance()->load_image(*tile);
    tiles[id] = tile;
    return tile;
}

std::string TileFactory::tile_id(uint16_t zoom, uint64_t x, uint64_t y) {
    std::stringstream ss;
    ss << zoom << "/" << x << "/" << y;
    return ss.str();
}
