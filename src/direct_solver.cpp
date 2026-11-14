#include <nbody/direct_solver.hpp>
#include "cuda/direct_nbody_kernel.cuh"
#include <stdexcept>
#include <string>

DirectSolver::DirectSolver(float softening) : m_softening(softening) {}

void DirectSolver::step(const BodyBuffers& read, BodyBuffers& write,
              const std::vector<glm::vec4>& host_positions, float dt) {
    cudaError_t err = direct_nbody_step(read.pos, read.vel, write.pos, write.vel, dt, read.n, m_softening);
    if (err != cudaSuccess)
        throw std::runtime_error(std::string("direct_nbody_step: ") + cudaGetErrorString(err));
}

void DirectSolver::compute_energy(const BodyBuffers& read, const DeviceBuffer<double>& energy, const std::vector<glm::vec4>& host_positions, float dt) {
    cudaError_t err = direct_compute_energy(read.pos, read.vel, read.n, m_softening, energy.get());
    if (err != cudaSuccess)
        throw std::runtime_error(std::string("direct_compute_energy: ") + cudaGetErrorString(err));
}