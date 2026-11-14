#pragma once
#include <vector>
#include <nbody/device_buffer.hpp>
#include <glm/glm.hpp>
#include <vector_types.h>

// Non-owning view of one set of device body arrays. Owned by Simulator.
struct BodyBuffers {
    float4* pos = nullptr; // xyz + mass
    float3* vel = nullptr;
    int n = 0;
};

class ForceSolver {
public:
    virtual ~ForceSolver() = default;

    // True if the solver benefits from bodies being kept in Morton (spatial) order,
    // e.g. tree solvers. Simulator only re-sorts bodies when this returns true.
    virtual bool wantsSortedBodies() const { return false; }

    virtual void step(const BodyBuffers& read, BodyBuffers& write,
                      const std::vector<glm::vec4>& host_positions, float dt) = 0;
    virtual void compute_energy(const BodyBuffers& read, const DeviceBuffer<double>& energy, const std::vector<glm::vec4>& host_positions, float dt) = 0;
};
