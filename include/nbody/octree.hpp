#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <vector_types.h>     // float4 / float3, identical layout on host and device
#include <vector_functions.h>

struct OctreeNode {            // alignment 16 (because of float4)
    float4 comMass;            // offset  0, 16 B  (xyz = COM, w = mass)
    float3 center;             // offset 16, 12 B
    float  halfWidth;          // offset 28,  4 B  <- fills the 4 B after float3
    int    child[8];           // offset 32, 32 B
    int    body;               // offset 64,  4 B
    int    isLeaf;             // offset 68, 1 = no children (single body or MAX_DEPTH bucket)
    int    _pad[2];            // offset 72,  8 B  <- explicit padding for alignment to 16 B
};  
static_assert(sizeof(OctreeNode) == 80, "OctreeNode must match the device layout");
static_assert(alignof(OctreeNode) == 16);
static_assert(offsetof(OctreeNode, child) == 32);
static_assert(offsetof(OctreeNode, isLeaf) == 68);

class Octree {
public:
    Octree() = default;

    static Octree build(const std::vector<glm::vec4>& positions);

    // Rebuild in place, reusing the node array's capacity (no reallocation after the first call).
    void rebuild(const std::vector<glm::vec4>& positions);

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
    void insert(std::int32_t bodyIdx, const std::vector<glm::vec4> &positions, std::int32_t nodeIdx, int depth = 0);
    std::size_t getOrCreateChild(std::int32_t parentIdx, std::int32_t octant);
};
