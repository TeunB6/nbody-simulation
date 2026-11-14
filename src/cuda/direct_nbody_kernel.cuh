#pragma once
#include <cuda_runtime.h>

cudaError_t direct_nbody_step(
    float4* positions_read, float3* velocities_read, 
    float4* positions_write, float3* velocities_write,
    float dt, int num_bodies, float softening);

cudaError_t direct_compute_energy(
    float4* positions, float3* velocities, int num_bodies,
     float softening, double* d_out /*2 doubles*/);