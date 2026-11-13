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
target_sources(main PRIVATE src/renderer.cpp src/simulator.cpp src/cuda/nbody_kernel.cu
               src/camera_controller.cpp src/initial_conditions.cpp
               src/octree.cpp src/direct_solver.cpp src/barnes_hut_solver.cpp
               src/cuda/barnes_hut_kernel.cu)
target_link_libraries(main PRIVATE glfw)
target_include_directories(main PRIVATE include)

# Dependencies not provided by the cluster modules are fetched at configure time.
# glad is generated here (needs python3 + jinja2); glm is header-only.
include(FetchContent)
FetchContent_Declare(glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    SOURCE_SUBDIR cmake)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    FIND_PACKAGE_ARGS NAMES glm)
FetchContent_MakeAvailable(glad glm)
glad_add_library(glad_gl_core_33 STATIC REPRODUCIBLE API gl:core=3.3)
target_link_libraries(main PRIVATE glad_gl_core_33 glm::glm)
