cmake_minimum_required(VERSION 3.22)

project(NBodySimulation LANGUAGES CXX C CUDA)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(main)
target_sources(main PRIVATE apps/main.cpp)
target_compile_definitions(main PRIVATE SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders")
find_package(CUDAToolkit REQUIRED)
target_link_libraries(main PRIVATE CUDA::cudart)
set_target_properties(main PROPERTIES CUDA_ARCHITECTURES "70;80;89")



find_package(glfw3 REQUIRED)
target_sources(main PRIVATE src/renderer.cpp src/simulator.cpp src/cuda/nbody_kernel.cu)
target_link_libraries(main PRIVATE glfw)
target_sources(main PRIVATE third_party/glad/src/gl.c)
target_include_directories(main PRIVATE include third_party/glad/include)
