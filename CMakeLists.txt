cmake_minimum_required(VERSION 3.22)

project(NBodySimulation LANGUAGES CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(main)
target_sources(main PRIVATE apps/main.cpp)
target_compile_definitions(main PRIVATE SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders")

find_package(glfw3 REQUIRED)
target_sources(main PRIVATE src/renderer.cpp src/simulator.cpp)
target_link_libraries(main PRIVATE glfw)
target_sources(main PRIVATE third_party/glad/src/gl.c)
target_include_directories(main PRIVATE include third_party/glad/include)
