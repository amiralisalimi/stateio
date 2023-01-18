cmake_minimum_required(VERSION 3.16)
project(state.io C)
set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE SOURCE "src/*.c" "src/*.h")
add_executable(state.io "${SOURCE}")

include_directories(
    "/usr/include/SDL2"
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(state.io m SDL2 SDL2_gfx SDL2_ttf SDL2_image)