project(slippymap3d)

cmake_minimum_required(VERSION 2.8.12)

# Conan Preamble
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# By default look for modules (e.g. Findsdl2.cmake) in the cmake-folder
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost COMPONENTS REQUIRED filesystem system asio thread)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(IlmBase REQUIRED)

#message(STATUS "Boost libraries: " ${Boost_LIBRARIES})

set (CMAKE_CXX_STANDARD 11)

# We need C++11
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g -std=gnu++0x")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${SDL2_IMAGE_INCLUDE_DIR})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${CURL_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ILMBASE_INCLUDE_DIRS})
include_directories(src)

# All source files from the current directory will be used
aux_source_directory(src SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${SDL2_IMAGE_LIBRARY} SDL2main SDL2
    ${JPEG_LIBRARY} ${PNG_LIBRARY}
    ${GLEW_LIBRARY}
    ${CURL_LIBRARY} 
    ${ZLIB_LIBRARY} 
    ${ILMBASE_Imath_LIBRARY}
    dl)

if(UNIX AND NOT APPLE)
target_link_libraries(${PROJECT_NAME} pthread)
endif()

