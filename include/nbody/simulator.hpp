#pragma once
#include <vector>
#include <cstddef>
#include <vector_types.h>


#define DIM 3

class Simulator {
public:
    Simulator(int n, float particle_mass, float softening);
    ~Simulator();

    void step(float dt);
    const float* positions();
    const float* velocities();
    const std::size_t count();
    const float3 centerOfMass();

private:
    std::vector<float4> m_pos;
    std::vector<float3> m_vel;

    float4 *d_pos_read = nullptr;
    float3 *d_vel_read = nullptr;
    float4 *d_pos_write = nullptr;
    float3 *d_vel_write = nullptr;
    std::size_t m_n;
    float m_softening = 0;
};