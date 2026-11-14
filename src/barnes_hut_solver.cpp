#include <nbody/barnes_hut_solver.hpp>
#include "cuda/barnes_hut_kernel.cuh"
#include <stdexcept>
#include <string>

BarnesHutSolver::BarnesHutSolver(float softening, float theta) : m_softening(softening), m_theta(theta) {}

void BarnesHutSolver::step(const BodyBuffers& read, BodyBuffers& write,
              const std::vector<glm::vec4>& host_positions, float dt) {
    m_tree.rebuild(host_positions);

    if (m_tree.nodeCount() > d_tree.size()) {
        d_tree.resize(m_tree.nodeCount() * 1.5);
    }
    d_tree.copyFromHost(m_tree.data(), m_tree.nodeCount());
    cudaError_t err = barnes_hut_step(read.pos, read.vel, write.pos, write.vel,
         d_tree.get(), m_tree.nodeCount(), dt, read.n, m_softening, m_theta);
    if (err != cudaSuccess)
        throw std::runtime_error(std::string("barnes_hut_step: ") + cudaGetErrorString(err));
}

void BarnesHutSolver::compute_energy(const BodyBuffers& read, const DeviceBuffer<double>& energy,
     const std::vector<glm::vec4>& host_positions, float dt)  {
    m_tree.rebuild(host_positions);
    if (m_tree.nodeCount() > d_tree.size()) {
        d_tree.resize(m_tree.nodeCount() * 1.5);
    }
    d_tree.copyFromHost(m_tree.data(), m_tree.nodeCount());
    cudaError_t err = barnes_hut_energy(read.pos, read.vel, energy.get(),
     d_tree.get(), m_tree.nodeCount(), read.n, m_softening, m_theta);
    if (err != cudaSuccess)
        throw std::runtime_error(std::string("barnes_hut_energy: ") + cudaGetErrorString(err));
}