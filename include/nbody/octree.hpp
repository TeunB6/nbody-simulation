#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

struct OctreeNode {
    glm::vec4 comMass;       // xyz = centre of mass, w = total mass
    glm::vec3 center;         
    float  halfWidth;      
    std::int32_t child[8]; // index into the node array, -1 = no child
    std::int32_t body;     // body index if leaf with one body, else -1
};

class Octree {
public:
    Octree() = default;

    static Octree build(const std::vector<glm::vec4>& positions);

    // Queries
    const OctreeNode& root() const;
    const OctreeNode& node(std::size_t i) const;
    std::size_t nodeCount() const;
    float totalMass() const;
    glm::vec3 centerOfMass() const;

    // Raw access for host-to-device copies
    const OctreeNode* data() const;

    // Debug only: child indices in range, mass sums match, etc.
    bool validate() const;

private:
    std::vector<OctreeNode> m_nodes; // m_nodes[0] is the root
    void insert(std::size_t bodyIdx, const std::vector<glm::vec4> &positions, std::size_t nodeIdx, int depth = 0);
    std::size_t getOrCreateChild(std::int32_t parentIdx, std::int32_t octant);
};
