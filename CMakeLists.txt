cmake_minimum_required(VERSION 3.16)
project(overlay_icon_repro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(Qt5 REQUIRED COMPONENTS Core Gui)

set(PLASMA_OVERLAY_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/plasma-workspace/applets/systemtray/statusnotifieritemsource.cpp
)
set(GENERATED_OVERLAY_SOURCE
    ${CMAKE_CURRENT_BINARY_DIR}/statusnotifieritemsource_overlayicon.cpp
)

add_custom_command(
    OUTPUT ${GENERATED_OVERLAY_SOURCE}
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/extract_overlay_icon.py
            ${PLASMA_OVERLAY_SOURCE}
            ${GENERATED_OVERLAY_SOURCE}
    DEPENDS
        ${PLASMA_OVERLAY_SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/extract_overlay_icon.py
        ${CMAKE_CURRENT_SOURCE_DIR}/src/statusnotifieritemsource_overlayicon.h
    VERBATIM
)

add_executable(overlay_icon_repro
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${GENERATED_OVERLAY_SOURCE}
)

target_include_directories(overlay_icon_repro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(overlay_icon_repro PRIVATE
    Qt5::Core
    Qt5::Gui
)
