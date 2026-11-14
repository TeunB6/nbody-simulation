#include <nbody/simulator.hpp>
#include <nbody/morton.hpp>
#include <cuda_runtime.h>          // cudaMalloc, cudaMemcpy, cudaFree, cudaGetLastError
#include <ranges>

glm::vec3 massWeightedDir(const std::vector<glm::vec4> &x) {
    glm::vec3 c = {0,0,0}; float m = 0;
    for (auto& p : x) {
        c.x += p.x*p.w; 
        c.y += p.y*p.w;
        c.z += p.z*p.w;
        m += p.w;
    }
    c.x /= m; c.y /= m; c.z /= m;
    return c;
}

glm::vec3 massWeightedDir(const std::vector<glm::vec3> &dir, const std::vector<glm::vec4> &mass_container) {
    glm::vec3 c = {0,0,0}; float m = 0;
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

Simulator::Simulator(const Bodies& initial, std::unique_ptr<ForceSolver> solver)
    : m_solver(std::move(solver)), m_pos(initial.pos), m_vel(initial.vel), m_n(m_pos.size())
{
    if (!m_solver)
        throw std::invalid_argument("Simulator requires a non-null force solver.");
    m_pos = initial.pos;
    m_vel = initial.vel;
    m_pos_dirty = false;
    m_vel_dirty = false;


    if (m_pos.size() != m_vel.size()) 
        throw std::invalid_argument("Initial configuration contains an inconsistant amount of bodies.");
    m_n = m_pos.size();
    d_pos.resize(m_n);
    d_vel.resize(m_n);
    d_pos_write.resize(m_n);
    d_vel_write.resize(m_n);
    d_energy.resize(2); // 2 doubles for kinetic and potential energy

    refreshViews();

    // Copy initial data to device
    d_pos.copyFromHost(reinterpret_cast<float4*>(m_pos.data()), m_n);
    d_vel.copyFromHost(reinterpret_cast<float3*>(m_vel.data()), m_n);

    if (m_solver->wantsSortedBodies())
        reorderBodies(); // start from a spatially sorted layout
}

void Simulator::refreshViews() {
    d_read  = { d_pos.get(),       d_vel.get(),       static_cast<int>(m_n) };
    d_write = { d_pos_write.get(), d_vel_write.get(), static_cast<int>(m_n) };
}

void Simulator::reorderBodies() {
    if (m_n < 2) return;
    // The host copies must be current before we permute them and upload them back.
    syncPositions();
    syncVelocities();
    auto [center, halfWidth] = boundingBox(m_pos);
    std::vector<int32_t> order = mortonOrder(m_pos, center, halfWidth);
    std::vector<glm::vec4> newPos(m_n);
    std::vector<glm::vec3> newVel(m_n);
    for (std::size_t i = 0; i < m_n; ++i) {
        newPos[i] = m_pos[order[i]];
        newVel[i] = m_vel[order[i]];
    }
    m_pos = std::move(newPos);
    m_vel = std::move(newVel);

    d_pos.copyFromHost(reinterpret_cast<float4*>(m_pos.data()), m_n);
    d_vel.copyFromHost(reinterpret_cast<float3*>(m_vel.data()), m_n);
    m_rebuild_counter = 0;
}

void Simulator::syncPositions() {
    if (!m_pos_dirty) return;
    d_pos.copyToHost(reinterpret_cast<float4*>(m_pos.data()), m_n);
    m_pos_dirty = false;
}

void Simulator::syncVelocities() {
    if (!m_vel_dirty) return;
    d_vel.copyToHost(reinterpret_cast<float3*>(m_vel.data()), m_n);
    m_vel_dirty = false;
}

const float* Simulator::positions() {
    syncPositions();
    return reinterpret_cast<const float*>(m_pos.data());
}

const float Simulator::virialRatio() {
    syncPositions();
    m_solver->compute_energy(d_read, d_energy, m_pos, 0.0f);
    double h[2];
    d_energy.copyToHost(h, 2);
    if (fabs(h[1]) < 1e-10) {
        throw std::runtime_error("Potential energy is too small, virial ratio is undefined.");
    }
    return 2 * h[0] / fabs(h[1]);
}

const float* Simulator::velocities() {
    syncVelocities();
    return reinterpret_cast<const float*>(m_vel.data());
}

const glm::vec3 Simulator::centerOfMass() {
    syncPositions();
    return massWeightedDir(m_pos);
}
 
const std::size_t Simulator::count() {
    return m_pos.size();
}

void Simulator::step(float dt) {
    syncPositions();
    if (m_solver->wantsSortedBodies() && m_rebuild_counter >= kReorderInterval) {
        reorderBodies(); // resets the counter
    }
    m_solver->step(d_read, d_write, m_pos, dt);

    // Swap the owning buffers
    d_pos.swap(d_pos_write);
    d_vel.swap(d_vel_write);
    refreshViews();
    m_pos_dirty = true;
    m_vel_dirty = true;
    ++m_rebuild_counter;
}
