// include/nbody/bodies.hpp
#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Bodies {
    std::vector<glm::vec4> pos;   // xyz + mass
    std::vector<glm::vec3> vel;
};
