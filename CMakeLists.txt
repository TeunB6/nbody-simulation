cmake_minimum_required(VERSION 3.31)

project(NBodySimulation)

add_executable(main)
target_sources(main PRIVATE apps/main.cpp)

find_package(glfw3 REQUIRED)
target_sources(main PRIVATE src/renderer.cpp)
target_link_libraries(main PRIVATE glfw)
target_sources(main PRIVATE third_party/glad/src/gl.c)
target_include_directories(main PRIVATE include third_party/glad/include)
