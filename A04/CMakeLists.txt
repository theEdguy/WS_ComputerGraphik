cmake_minimum_required (VERSION 3.9)
project (raytracer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-g -Wall -Wextra -Wpedantic)

# SDL2 finden
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(raytracer 
    raytracer.cc 
    math.cc 
    geometry.cc
)

# SDL2 linken
target_link_libraries(raytracer ${SDL2_LIBRARIES})