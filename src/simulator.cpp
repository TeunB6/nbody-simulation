#include <nbody/simulator.hpp>
#include <random>
#include <cuda_runtime.h>          // cudaMalloc, cudaMemcpy, cudaFree, cudaGetLastError
#include "cuda/nbody_kernel.cuh"   // declaration of run_nbody_step
#include <system_error>
#include <ranges>

#define CUDA_CHECK(x) do { cudaError_t e = (x); if (e != cudaSuccess) \
    throw std::runtime_error(std::string(cudaGetErrorString(e)) + " at " + __FILE__ + ":" + std::to_string(__LINE__)); \
 } while (0)


float3 mass_weighted_dir(const std::vector<float4> &x) {
    float3 c = {0,0,0}; float m = 0;
    for (auto& p : x) {
        c.x += p.x*p.w; 
        c.y += p.y*p.w;
        c.z += p.z*p.w;
        m += p.w;
    }
    c.x /= m; c.y /= m; c.z /= m;
    return c;
}

float3 mass_weighted_dir(const std::vector<float3> &dir, const std::vector<float4> &mass_container) {
    float3 c = {0,0,0}; float m = 0;
    if (dir.size() != mass_container.size())
        throw std::invalid_argument("Mass containing and direction vectors are not of matching length\n");
    
    for (size_t i = 0; i < dir.size(); ++i) {
        c.x += dir[i].x*mass_container[i].w; 
        c.y += dir[i].y*mass_container[i].w;
        c.z += dir[i].z*mass_container[i].w;
        m += mass_container[i].w;
    }
    c.x /= m; c.y /= m; c.z /= m;
    return c;
}

Simulator::Simulator(int n, float particle_mass, float softening):
     m_n(n), m_pos(n), m_vel(n), m_softening(softening) 
{
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> d(-1.f, 1.f);

    for (std::size_t i = 0; i < n; ++i) {
        m_pos[i].x = d(rng);
        m_pos[i].y = d(rng);
        m_pos[i].z = 0;  // 2d for now
        m_pos[i].w = particle_mass;

        m_vel[i].x = d(rng);
        m_vel[i].y = d(rng);
        m_vel[i].z = 0; // 2d for now
    }

    // Make sure net momentum is 0 at the start
    float3 net_momentum = mass_weighted_dir(m_vel, m_pos);
    for (std::size_t i = 0; i < n; ++i) {
        m_vel[i].x -= net_momentum.x;
        m_vel[i].y -= net_momentum.y;
        m_vel[i].z -= net_momentum.z;
    }

    // TODO: finda way to use RAII on these buffers as they are not freed if the constructor fails mid construction.
    // read arrays 
    CUDA_CHECK(cudaMalloc(&d_pos_read, sizeof(float4) * n));
    CUDA_CHECK(cudaMalloc(&d_vel_read, sizeof(float3) * n));

    // write arrays
    CUDA_CHECK(cudaMalloc(&d_pos_write, sizeof(float4) * n));
    CUDA_CHECK(cudaMalloc(&d_vel_write, sizeof(float3) * n));

    CUDA_CHECK(cudaMemcpy(d_pos_read, m_pos.data(), n * sizeof(float4), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_vel_read, m_vel.data(), n * sizeof(float3), cudaMemcpyHostToDevice));
}

const float* Simulator::positions() {
    CUDA_CHECK(cudaMemcpy(m_pos.data(), d_pos_read, m_n * sizeof(float4), cudaMemcpyDeviceToHost));
    return reinterpret_cast<const float*>(m_pos.data());
}

const float* Simulator::velocities() {
    CUDA_CHECK(cudaMemcpy(m_vel.data(), d_vel_read, m_n * sizeof(float3), cudaMemcpyDeviceToHost));
    return reinterpret_cast<const float*>(m_vel.data());
}

const float3 Simulator::centerOfMass() {
    return mass_weighted_dir(m_pos);
}
 
const std::size_t Simulator::count() {
    return m_pos.size();
}
void Simulator::step(float dt) {
    CUDA_CHECK(run_nbody_step(d_pos_read, d_vel_read, d_pos_write, d_vel_write, dt, m_n, m_softening));
    // swap for next iteration
    std::swap(d_pos_read, d_pos_write);
    std::swap(d_vel_read, d_vel_write);
}


Simulator::~Simulator() {
    cudaFree(d_pos_read);
    cudaFree(d_pos_write);
    cudaFree(d_vel_read);
    cudaFree(d_vel_write);
}