#pragma once
#include <vector>
#include <nbody/bodies.hpp>
#include <cstddef>
#include <glm/glm.hpp>
#include <vector_types.h>


#define DIM 3

class Simulator {
public:
    Simulator(const Bodies& initial, float softening);
    ~Simulator();

    void step(float dt);
    const float* positions();
    const float* velocities();
    const std::size_t count();
    const glm::vec3 centerOfMass();
    const float virialRatio();

private:
    std::vector<glm::vec4> m_pos;
    std::vector<glm::vec3> m_vel;

    float4 *d_posRead = nullptr;
    float3 *d_velRead = nullptr;
    float4 *d_posWrite = nullptr;
    float3 *d_velWrite = nullptr;

    double *d_energy = nullptr;

    std::size_t m_n;
    float m_softening = 0;
};