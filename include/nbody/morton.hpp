#pragma once
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

int32_t spreadBits10(uint32_t x);
int32_t computeMortonCode(const glm::vec4& position, const glm::vec3& center, float halfWidth);
std::vector<int32_t> mortonOrder(const std::vector<glm::vec4>& positions, const glm::vec3& center, float halfWidth);
std::pair<glm::vec3,float> boundingBox(const std::vector<glm::vec4>& positions);