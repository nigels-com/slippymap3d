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
#include <boost/filesystem.hpp>
#include <SDL2/SDL_image.h>
#include <curl/curl.h>
#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>

#include <memory>

#include "loader.h"
#include "global.h"

static size_t count = 0;
static boost::asio::io_service       * service = NULL;
static boost::thread_group           * pool    = NULL;
static boost::asio::io_service::work * work    = NULL;

//Loader* Loader::_instance = nullptr;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void Loader::start()
{
    ++count;
    if (count==1)
    {
        service = new boost::asio::io_service();
        work = new boost::asio::io_service::work(*service);
        pool = new boost::thread_group();
        const size_t threads = 10;
        for (size_t i = 0; i < threads; i++) {
            pool->create_thread(boost::bind(&boost::asio::io_service::run, service));
        }
    }
}

void Loader::stop()
{
    --count;
    if (count==0)
    {
        service->stop();
        pool->join_all();
        delete pool;
        delete work;
        delete service;
    }
}

void Loader::download_image(Tile* tile)
{
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return;
    }

    std::stringstream dirname;
    dirname << m_dir << tile->zoom << "/" << (m_zxy ? tile->x : (m_tms ? tile->y : (uint64_t(1)<<tile->zoom) - 1 - tile->y));
    std::string dir = dirname.str();
    boost::filesystem::create_directories(dir);
    std::string filename = tile->get_filename(m_tms, m_zxy, m_extension);
    std::string url = m_prefix + filename;    
    std::string file = m_dir + filename;
    FILE* fp = fopen(file.c_str(), "wb");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    CURLcode res = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        std::cerr << "Failed to download: " << url << " " << res << std::endl;
    } else {
        tile->texid = 0;
    }
}

void Loader::load_image(Tile& tile)
{
    std::string filename = m_dir + tile.get_filename(m_tms, m_zxy, m_extension);
    if (!boost::filesystem::exists(filename)) {
        service->post(boost::bind(&Loader::download_image, this, &tile));
        return;
    }
    if (boost::filesystem::file_size(filename) == 0) {
        boost::filesystem::remove(filename);
        service->post(boost::bind(&Loader::download_image, this, &tile));
        return;
    }

    open_image(tile);
}

void Loader::open_image(Tile &tile)
{
    std::string filename = m_dir + tile.get_filename(m_tms, m_zxy, m_extension);
    SDL_Surface *texture = IMG_Load(filename.c_str());

    char tmp[4096];
    getcwd(tmp, 4096);
    std::cout << "Loading texture " << filename << " from directory " << tmp << ' ';

    if (texture) {
        if (SDL_MUSTLOCK(texture))
        {
            SDL_LockSurface(texture);
        }

        GLenum format;
        GLint internalFormat;
        if (texture->format->BytesPerPixel == 4) {
            if (texture->format->Rmask == 0x000000ff) {
                format = GL_RGBA;
                internalFormat = GL_RGBA8;
            } else {
                format = GL_BGRA;
                internalFormat = GL_RGBA8;
            }
        } else if (texture->format->BytesPerPixel == 3) {
            if (texture->format->Rmask == 0x000000ff) {
                format = GL_RGB;
                internalFormat = GL_RGB8;
            } else {
                format = GL_BGR;
                internalFormat = GL_RGB8;
            }
        } else {
//            std::cout << "INVALID (" << SDL_GetPixelFormatName(texture->format->format);
            SDL_PixelFormat* pformat = SDL_AllocFormat(SDL_PIXELFORMAT_BGR24);
            SDL_Surface* tmp = SDL_ConvertSurface(texture, pformat, 0);
            format = GL_BGR;
            internalFormat = GL_RGB8;
            if (SDL_MUSTLOCK(texture)) {
                SDL_UnlockSurface(texture);
            }
            SDL_FreeSurface(texture);
            texture = tmp;
            if (SDL_MUSTLOCK(texture)) {
                SDL_LockSurface(texture);
            }
//            std::cout << " -> " << SDL_GetPixelFormatName(texture->format->format) << ") ";
        }

        GLuint texid;
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texture->w, texture->h, 0, format, GL_UNSIGNED_BYTE, texture->pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (SDL_MUSTLOCK(texture)) {
            SDL_UnlockSurface(texture);
        }
        SDL_FreeSurface(texture);

        tile.texid = texid;

        std::cout << "SUCCESS" << std::endl;
    } else {
        tile.texid = TileFactory::instance()->get_dummy();
        std::cout << "FAILED" << std::endl;
    }
}
