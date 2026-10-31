#pragma once
#include <glm/glm.hpp>
#include <nbody/bodies.hpp>
#include <vector>
#include <cstdint>
#include <utility>
#include <optional>
#include <string_view>


enum class InitMethod {Uniform, Plummer, Disk, Collision, Collapse};
std::optional<InitMethod> parseInitMethod(std::string_view name);

Bodies makeBodies(InitMethod method, size_t n, std::uint32_t seed);

Bodies makeUniform(size_t n, std::uint32_t seed);
Bodies makePlummer(size_t n, std::uint32_t seed);
Bodies makeDisk(size_t n, std::uint32_t seed, float centralMass = 0.1f);
Bodies makeCollision(size_t n, std::uint32_t seed);
Bodies makeCollapse(size_t n, std::uint32_t seed);


void recenter(Bodies& b);   // zero total momentum and centre of mass
