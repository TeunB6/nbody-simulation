#pragma once
#include <nbody/force_solver.hpp>

class DirectSolver : public ForceSolver {
public:
    explicit DirectSolver(float softening);
    void step(const BodyBuffers& read, BodyBuffers& write,
              const std::vector<glm::vec4>& host_positions, float dt) override;
    void compute_energy(const BodyBuffers& read, const DeviceBuffer<double>& energy,
                        const std::vector<glm::vec4>& host_positions, float dt) override;

private:
    float m_softening;
    // TODO: members
};
