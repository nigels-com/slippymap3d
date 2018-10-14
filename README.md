slippymap3d
===========

This is a heavily modified fork of egore/slippymap3d

Building
--------

slippymap3d uses Conan and CMake to compile the code.

System setup:

    $ sudo apt install build-essentials git python-pip
    $ pip install conan

Code repository:

    $ git clone https://github.com/nigels-com/slippymap3d.git

Building the program:

    $ cd slippymap3d
    $ mkdir build
    $ cd build
    $ git clean -xdf . && conan install .. --build=missing && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j4

CMake Debug or Release mode:

	$ cmake -DCMAKE_BUILD_TYPE=Debug ..
	$ cmake -DCMAKE_BUILD_TYPE=Release ..

Conan Debug or Release mode:

    $ conan install .. -s build_type=Debug
    $ conan install .. -s build_type=Release

Cross-building 32-bit binaries on Ubuntu:

    $ git clean -xdf . && conan install .. -s arch=x86 --build=missing && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-m32 .. && make -j4

Mac build:

    git clean -xdf . && conan install .. --build=missing -s compiler.version=10.0 && cmake .. && make -j4


Using official tiles
--------------------

Starting the program will try to download tiles from "http://localhost/osm_tiles/" to
*the current directory*. If you want to use the official tiles from OpenStreetMap (note
that you need to follow the tile usage policy) replace the URL in loader.cpp with e.g.
"http://a.tile.openstreetmap.org/".

Keyboard
--------

* *g* to toggle tile grid lines
* *c* to toggle center cross
* *i* or *o* to zoom
* *left*, *right*, *up*, *down* arrows to pan

Mouse
-----

* Left mouse button: panning
* Right mouse button: zooming
* Middle mouse wheel: zooming

Screenshots
-----------

To see a few screenshots take a look at https://plus.google.com/+ChristophBrill/posts/CLife8pvGoB
