cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-static")

project(Tetrish LANGUAGES CXX VERSION 1.0.0)

add_compile_options(-O3)

file(GLOB SRC src/engine/*.h src/engine/bridge/*.h src/system/*.h src/main.cpp src/*.h)
file(GLOB SFINDER_CPP sfinder-cpp/core/*.cpp sfinder-cpp/core/*.hpp sfinder-cpp/finder/*.cpp sfinder-cpp/finder/*.hpp)

add_executable(Tetrish ${SRC} ${SFINDER_CPP})

find_package(fmt CONFIG REQUIRED)
target_link_libraries(Tetrish PRIVATE fmt::fmt-header-only)
target_compile_options(Tetrish PRIVATE -w)