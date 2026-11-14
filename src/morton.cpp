#include <nbody/morton.hpp>
#include <algorithm>
#include <limits>
#include <stdexcept>

int32_t spreadBits10(uint32_t x) { // spread the lower 10 bits of x so that there are 2 zeros between each bit
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;
    return static_cast<int32_t>(x);
}

int32_t computeMortonCode(const glm::vec4& position, const glm::vec3& center, float halfWidth) {
    glm::vec3 normalizedPos = (glm::vec3(position) - center) / (2.0f * halfWidth) + 0.5f;
    normalizedPos = glm::clamp(normalizedPos, glm::vec3(0.0f), glm::vec3(1.0f));

    // Scale to [0, 1023] for 10 bits
    uint32_t x = static_cast<uint32_t>(normalizedPos.x * 1023.0f);
    uint32_t y = static_cast<uint32_t>(normalizedPos.y * 1023.0f);
    uint32_t z = static_cast<uint32_t>(normalizedPos.z * 1023.0f);
    // 30 bit key
    return (spreadBits10(x) << 2) | (spreadBits10(y) << 1) | spreadBits10(z);
}

std::vector<int32_t> mortonOrder(const std::vector<glm::vec4>& positions, const glm::vec3& center, float halfWidth) {
    std::vector<std::pair<int32_t, int32_t>> mortonCodes(positions.size());
    for (size_t i = 0; i < positions.size(); ++i) {
        mortonCodes[i] = {computeMortonCode(positions[i], center, halfWidth), static_cast<int32_t>(i)};
    }
    std::sort(mortonCodes.begin(), mortonCodes.end());
    std::vector<int32_t> order(positions.size());
    for (size_t i = 0; i < positions.size(); ++i) {
        order[i] = mortonCodes[i].second;
    }
    return order;
}

std::pair<glm::vec3,float> boundingBox(const std::vector<glm::vec4>& positions) {
    if (positions.empty()) throw std::invalid_argument("positions should not be empty");
    glm::vec3 pMin = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    glm::vec3 pMax = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
    for (auto const& p : positions) {
        pMin.x = std::min(pMin.x, p.x);
        pMin.y = std::min(pMin.y, p.y);
        pMin.z = std::min(pMin.z, p.z);

        pMax.x = std::max(pMax.x, p.x);
        pMax.y = std::max(pMax.y, p.y);
        pMax.z = std::max(pMax.z, p.z);
    }

    glm::vec3 center = {
        (pMin.x + pMax.x) / 2,
        (pMin.y + pMax.y) / 2,
        (pMin.z + pMax.z) / 2,
    };
    float halfWidth = std::max(pMax.x - center.x, std::max(pMax.y - center.y, pMax.z - center.z));
    return {center, halfWidth};
}   