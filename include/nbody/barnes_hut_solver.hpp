#pragma once
#include <nbody/force_solver.hpp>
#include <nbody/device_buffer.hpp>
#include <nbody/octree.hpp>


// Owns device-side tree buffers and launches the traversal kernel.
class BarnesHutSolver : public ForceSolver {
public:
    explicit BarnesHutSolver(float softening, float theta);
    void step(const BodyBuffers& read, BodyBuffers& write,
              const std::vector<glm::vec4>& host_positions, float dt) override;
    void compute_energy(const BodyBuffers& read, const DeviceBuffer<double>& energy,
                        const std::vector<glm::vec4>& host_positions, float dt) override;
    bool wantsSortedBodies() const override { return true; }

private:
    float m_softening;
    float m_theta;
    Octree m_tree;                  // reused every step so its node array keeps its capacity
    DeviceBuffer<OctreeNode> d_tree;
};
