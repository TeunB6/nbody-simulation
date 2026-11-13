#include <nbody/octree.hpp>
#include <utility>
#include <limits>
#include <tuple>
#include <stdexcept>

static constexpr int MAX_DEPTH = 32;


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

bool insideBoundingBox(const glm::vec3& center, float halfwidth, const glm::vec4& point) {
    return (point.x < center.x + halfwidth && point.x > center.x - halfwidth &&
        point.y < center.y + halfwidth && point.y > center.y - halfwidth &&
        point.z < center.z + halfwidth && point.z > center.z - halfwidth);
}

std::size_t Octree::getOrCreateChild(std::int32_t parentIdx, std::int32_t octant) {
    std::size_t existingChildIdx = m_nodes[parentIdx].child[octant];
    if (existingChildIdx != -1) {
        return existingChildIdx;
    }

    float childHalfWidth = m_nodes[parentIdx].halfWidth * 0.5;
    glm::vec3 parentCenter = m_nodes[parentIdx].center;

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
    m_nodes.push_back(
        {
            {0.0, 0.0, 0.0, 0.0}, // CoM, total mass
            childCenter, // center 
            childHalfWidth, // halfwidth 
            {-1,-1,-1,-1,-1,-1,-1,-1}, // child indices
            -1, // body
        }
    );
    m_nodes[parentIdx].child[octant] = childIdx;
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

void Octree::insert(std::size_t bodyIdx, const std::vector<glm::vec4> &positions, std::size_t nodeIdx, int depth) {
    if (depth > MAX_DEPTH) throw std::runtime_error("Octree::insert exceeded MAX_DEPTH (coincident bodies?)");
    glm::vec4 pos = positions[bodyIdx];
    // CASE 1: Node is empty -> make it a leaf
    if (m_nodes[nodeIdx].comMass.w == 0.0) {
        m_nodes[nodeIdx].comMass = glm::vec4(glm::vec3(pos) * pos.w, pos.w); // mass-weighted sum, normalized in build()
        m_nodes[nodeIdx].body = bodyIdx;
        return;
    }

    // CASE 2: Node is leaf -> make it internal and make child leaf
    if (m_nodes[nodeIdx].body != -1 && m_nodes[nodeIdx].comMass.w > 0.0) {
        std::size_t oldBodyIdx = m_nodes[nodeIdx].body;
        m_nodes[nodeIdx].body = -1; 

        std::size_t oldOctant = getChildOctant(m_nodes[nodeIdx].center, positions[oldBodyIdx]);
        std::size_t childIdx = getOrCreateChild(nodeIdx, oldOctant);
        insert(oldBodyIdx, positions, childIdx, depth + 1);
    }

    // CASE 3: Node is node is internal -> update and aggregate
    m_nodes[nodeIdx].comMass += glm::vec4(glm::vec3(pos) * pos.w, pos.w);

    int newOctant = getChildOctant(m_nodes[nodeIdx].center, pos);
    std::size_t childIdx = getOrCreateChild(nodeIdx, newOctant);
    insert(bodyIdx, positions, childIdx, depth + 1);   
}

Octree Octree::build(const std::vector<glm::vec4>& positions) {
    Octree tree;
    glm::vec3 center; float halfwidth;
    std::tie(center, halfwidth) = boundingBox(positions);
    tree.m_nodes.push_back(
        {
            {0.0, 0.0, 0.0, 0.0}, // CoM, total mass
            center, // center 
            halfwidth, // halfwidth 
            {-1,-1,-1,-1,-1,-1,-1,-1}, // child indices
            -1, // body
        }
    );

    for (std::int32_t i = 0; i < static_cast<std::int32_t>(positions.size()); ++i) {
        tree.insert(i, positions, 0);
    }

    // comMass holds (sum m*pos, sum m) during insertion; divide once per node here
    for (OctreeNode &node : tree.m_nodes) {
        if (node.comMass.w > 0.0f)
            node.comMass = glm::vec4(glm::vec3(node.comMass) / node.comMass.w, node.comMass.w);
    }
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
    return glm::vec3(m_nodes[0].comMass);
}

float Octree::totalMass() const {
    return m_nodes[0].comMass.w;
}

const OctreeNode* Octree::data() const {
    return m_nodes.data();
}

bool Octree::validate() const {
    for (const OctreeNode &node : m_nodes) {
        if (node.body != -1) continue;
        glm::vec4 expectedCoM = {0.0,0.0,0.0,0.0};
        for (std::int32_t childIdx : node.child) {
            if (childIdx == -1) continue;
            if (childIdx >= m_nodes.size()) return false;
            expectedCoM += m_nodes[childIdx].comMass * m_nodes[childIdx].comMass.w;
        } 
        expectedCoM /= expectedCoM.w; 

        if(node.comMass != expectedCoM) return false;
    }
    return true;
}
