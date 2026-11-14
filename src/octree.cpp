#include <nbody/octree.hpp>
#include <nbody/morton.hpp> // boundingBox
#include <utility>
#include <tuple>
#include <stdexcept>

/*
Possible improvments:
- Add rope skip pointers to tree for faster gpu traversal.
- 

*/

static constexpr int MAX_DEPTH = 32;

// OctreeNode uses float3/float4 (shared with the GPU); Builder same type as sim and renderer glm::vec3/vec4. 
static inline glm::vec3 toVec3(const float3& v) { return {v.x, v.y, v.z}; }
static inline glm::vec4 toVec4(const float4& v) { return {v.x, v.y, v.z, v.w}; }
static inline float3 toFloat3(const glm::vec3& v) { return make_float3(v.x, v.y, v.z); }
static inline float4 toFloat4(const glm::vec4& v) { return make_float4(v.x, v.y, v.z, v.w); }

static OctreeNode makeNode(const glm::vec3& center, float halfWidth) {
    OctreeNode n{};
    n.comMass = make_float4(0.f, 0.f, 0.f, 0.f);
    n.center = toFloat3(center);
    n.halfWidth = halfWidth;
    for (int& c : n.child) c = -1;
    n.body = -1;
    n.isLeaf = 1; // flipped to 0 in getOrCreateChild when the first child is added
    return n;
}


std::size_t Octree::getOrCreateChild(std::int32_t parentIdx, std::int32_t octant) {
    std::size_t existingChildIdx = m_nodes[parentIdx].child[octant];
    if (existingChildIdx != -1) {
        return existingChildIdx;
    }

    float childHalfWidth = m_nodes[parentIdx].halfWidth * 0.5;
    glm::vec3 parentCenter = toVec3(m_nodes[parentIdx].center);

    /* Octant layout
       0 1 
       2 3 / 
       4 5 
       6 7 

       0 = 000
       1 = 001
       2 = 010
       3 = 011
       4 = 100
       5 = 101
       6 = 110
       7 = 111
       
    */

    glm::vec3 offset = {
        (octant % 2 == 0) ? -childHalfWidth : childHalfWidth, // left half
        (octant >= 4) ? -childHalfWidth : childHalfWidth, // bottom
        (octant & 2) ? -childHalfWidth : childHalfWidth // front
    };

    glm::vec3 childCenter = parentCenter + offset;

    std::size_t childIdx = m_nodes.size();
    m_nodes.push_back(makeNode(childCenter, childHalfWidth));
    m_nodes[parentIdx].child[octant] = childIdx;
    m_nodes[parentIdx].isLeaf = 0;
    return childIdx;
};

std::int32_t getChildOctant(const glm::vec3& center, const glm::vec4& point) {
    /* Octant layout
       0 1 
       2 3 / 
       4 5 
       6 7 */
    std::int32_t octant = 0;
    if (point.y <= center.y) // bottom half (4-7)
        octant += 4;
    
    if (point.x >= center.x) // right half (1,3 or 5,7)
        octant += 1;
    
    if (point.z <= center.z) // front half (2,3 or 6,7) 
        octant += 2;
    return octant;
}

void Octree::insert(int32_t bodyIdx, const std::vector<glm::vec4> &positions, int32_t nodeIdx, int depth) {
    glm::vec4 pos = positions[bodyIdx];
    // CASE 1: Node is empty -> make it a leaf
    if (m_nodes[nodeIdx].comMass.w == 0.0 && m_nodes[nodeIdx].body == -1) {
        m_nodes[nodeIdx].comMass = toFloat4(glm::vec4(glm::vec3(pos) * pos.w, pos.w)); // mass-weighted sum, normalized in build()
        m_nodes[nodeIdx].body = bodyIdx;
        return;
    }

    // CASE 2: depth limit reached. Stop subdividing and
    // merge the body into this node, which becomes a multi-body "bucket" leaf: body = -1
    if (depth >= MAX_DEPTH) {
        m_nodes[nodeIdx].comMass = toFloat4(toVec4(m_nodes[nodeIdx].comMass) + glm::vec4(glm::vec3(pos) * pos.w, pos.w));
        m_nodes[nodeIdx].body = -1;
        return;
    }

    // CASE 3: Node is leaf -> make it internal and make child leaf
    if (m_nodes[nodeIdx].body != -1 && m_nodes[nodeIdx].comMass.w > 0.0) {
        int32_t oldBodyIdx = m_nodes[nodeIdx].body;
        m_nodes[nodeIdx].body = -1; 

        int32_t oldOctant = getChildOctant(toVec3(m_nodes[nodeIdx].center), positions[oldBodyIdx]);
        int32_t childIdx = getOrCreateChild(nodeIdx, oldOctant);
        insert(oldBodyIdx, positions, childIdx, depth + 1);
    }

    // CASE 4: Node is node is internal -> update and aggregate
    m_nodes[nodeIdx].comMass = toFloat4(toVec4(m_nodes[nodeIdx].comMass) + glm::vec4(glm::vec3(pos) * pos.w, pos.w));

    int newOctant = getChildOctant(toVec3(m_nodes[nodeIdx].center), pos);
    int32_t childIdx = getOrCreateChild(nodeIdx, newOctant);
    insert(bodyIdx, positions, childIdx, depth + 1);   
}

void Octree::rebuild(const std::vector<glm::vec4>& positions) {
    glm::vec3 center; float halfwidth;
    std::tie(center, halfwidth) = boundingBox(positions);
    m_nodes.clear(); // keeps capacity
    m_nodes.reserve(2 * positions.size() + 1); // ~2N nodes typical; no-op once capacity is enough
    m_nodes.push_back(makeNode(center, halfwidth));

    for (std::int32_t i = 0; i < static_cast<std::int32_t>(positions.size()); ++i) {
        insert(i, positions, 0);
    }

    // comMass holds (sum m*pos, sum m) during insertion; divide once per node here
    for (OctreeNode &node : m_nodes) {
        if (node.comMass.w > 0.0f)
            node.comMass = toFloat4(glm::vec4(glm::vec3(toVec4(node.comMass)) / node.comMass.w, node.comMass.w));
    }
}

Octree Octree::build(const std::vector<glm::vec4>& positions) {
    Octree tree;
    tree.rebuild(positions);
    return tree;
}

const OctreeNode &Octree::root() const {
    return m_nodes[0];
}

const OctreeNode &Octree::node(std::size_t i) const {
    return m_nodes[i];
}

std::size_t Octree::nodeCount() const {
    return m_nodes.size();
}

glm::vec3 Octree::centerOfMass() const {
    return glm::vec3(toVec4(m_nodes[0].comMass));
}

float Octree::totalMass() const {
    return m_nodes[0].comMass.w;
}

const OctreeNode* Octree::data() const {
    return m_nodes.data();
}

bool Octree::validate() const {
    for (const OctreeNode &node : m_nodes) {
        if (node.isLeaf) continue; // single-body leaf or MAX_DEPTH bucket
        glm::vec4 expectedCoM = {0.0,0.0,0.0,0.0};
        float expectedMass = 0.0f;
        for (std::int32_t childIdx : node.child) {
            if (childIdx == -1) continue;
            if (childIdx >= m_nodes.size()) return false;
            expectedCoM += toVec4(m_nodes[childIdx].comMass) * m_nodes[childIdx].comMass.w;
            expectedMass += m_nodes[childIdx].comMass.w;
        } 
        expectedCoM /= expectedCoM.w; 
        expectedCoM.w = expectedMass;

        if(glm::length(toVec4(node.comMass) - expectedCoM) > 1e-10) return false;
    }
    return true;
}
