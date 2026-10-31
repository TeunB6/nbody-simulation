cmake_minimum_required(VERSION 3.31)

project(NBodySimulation)

add_executable(main)
target_sources(main PRIVATE apps/main.cpp)
find_package(glfw3 REQUIRED)
target_sources(main PRIVATE src/renderer.cpp)
target_link_libraries(main PRIVATE glfw)
