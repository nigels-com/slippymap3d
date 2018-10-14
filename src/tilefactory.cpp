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

#include "tilefactory.h"
#include "loader.h"

TileFactory* TileFactory::_instance = nullptr;

TileFactory::~TileFactory() {
    for (const auto & i : tiles) {
        delete i.second;
    }
    tiles.clear();
}

Tile* TileFactory::get_tile(Loader & loader, uint16_t zoom, uint64_t x, uint64_t y) {
    std::string id = tile_id(zoom, x, y);
    auto key = std::make_pair(&loader, id);
    auto i = tiles.find(key);
    if (i != tiles.end()) {
        return i->second;
    }
    Tile* tile = new Tile(zoom, x, y, dummy);
    loader.load_image(*tile);
    tiles[key] = tile;
    return tile;
}

Tile* TileFactory::get_tile_at(Loader & loader, uint16_t zoom, uint64_t x, uint64_t y)
{
    if (zoom)
    {
        x >>= (64-zoom);
        y >>= (64-zoom);
    }
    else
    {
        x = y = 0;
    }
    std::string id = tile_id(zoom, x, y);
    auto key = std::make_pair(&loader, id);
    auto i = tiles.find(key);
    if (i != tiles.end()) {
        return i->second;
    }
    Tile* tile = new Tile(zoom, x, y, dummy);
    loader.load_image(*tile);
    tiles[key] = tile;
    return tile;
}

std::string TileFactory::tile_id(uint16_t zoom, uint64_t x, uint64_t y) {
    std::stringstream ss;
    ss << zoom << "/" << x << "/" << y;
    return ss.str();
}
