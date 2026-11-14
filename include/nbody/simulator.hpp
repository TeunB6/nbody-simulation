#pragma once
#include <vector>
#include <memory>
#include <nbody/bodies.hpp>
#include <nbody/force_solver.hpp>
#include <nbody/device_buffer.hpp>
#include <cstddef>
#include <glm/glm.hpp>
#include <vector_types.h>


#define DIM 3

class Simulator {
public:
    Simulator(const Bodies& initial, std::unique_ptr<ForceSolver> solver);
    ~Simulator() = default;

    void step(float dt);
    const float* positions();
    const float* velocities();
    const std::size_t count();
    const glm::vec3 centerOfMass();
    const float virialRatio();
    void reorderBodies();

private:
    void refreshViews();
    void syncPositions();
    void syncVelocities();

    std::unique_ptr<ForceSolver> m_solver;

    std::vector<glm::vec4> m_pos;
    std::vector<glm::vec3> m_vel;

    bool m_pos_dirty = true;
    bool m_vel_dirty = true;
    static constexpr std::size_t kReorderInterval = 10; // steps between Morton re-sorts
    std::size_t m_rebuild_counter = 0;

    DeviceBuffer<float4> d_pos;
    DeviceBuffer<float3> d_vel;
    DeviceBuffer<float4> d_pos_write;
    DeviceBuffer<float3> d_vel_write;

    BodyBuffers d_read;
    BodyBuffers d_write;

    DeviceBuffer<double> d_energy; // 2 doubles for kinetic and potential energy

    std::size_t m_n;
};