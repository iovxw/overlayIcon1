cmake_minimum_required(VERSION 3.16)
project(overlay_icon_repro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(overlay_icon_repro
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lodepng/lodepng.cpp
)

target_include_directories(overlay_icon_repro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lodepng
)
