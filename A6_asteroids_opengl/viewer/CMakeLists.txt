# ==============================================================
# Anleitung: Projekt mit CMake und g++ (Linux oder MSYS2/Windows) bauen
#
# 1. Erstelle ein Build-Verzeichnis (empfohlen):
#      mkdir build
#
# 2. Wechsle ins Build-Verzeichnis:
#      cd build
#
# 3. Konfiguriere das Projekt:
#      cmake ..
#
#    - Unter MSYS2 UCRT ggf. explizit Generator angeben:
#      cmake -G "MSYS Makefiles" ..
#
# 4. Baue das Projekt:
#      cmake --build .
#    oder besser vom übergeordneten Ordner
#      cmake --build build
#
# 5. Das Programm nicht im build-Verzeichnis, sondern im übergeordneten Ordner aufrufen:
#      ./build/viewer.exe
#
# Voraussetzungen:
#   - CMake installiert (mind. Version 3.10)
#   - g++ und benötigte Bibliotheken (z.B. GLM, SDL2, GLEW, OpenGL)
# ==============================================================

cmake_minimum_required(VERSION 3.10)
project(viewer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW OR MSYS OR WIN32)
    add_compile_options(-g -Wall -Wextra -Wpedantic)
    add_link_options("-Wl,--stack,16777216")
    add_executable(viewer viewer.cc wavefront.cc)
    target_link_libraries(viewer mingw32 SDL2main SDL2 OPENGL32 GLEW32)
    # target_link_options(viewer PRIVATE -mwindows) # falls GUI
else()
    add_compile_options(-g -Wall -Wextra -Wpedantic)
    add_executable(viewer viewer.cc wavefront.cc)
    target_link_libraries(viewer SDL2 GL GLEW)
endif()

add_executable(wavefront_test wavefront.cc wavefront_test.cc)
target_link_libraries(wavefront_test gtest gtest_main)
