#pragma once

cudaError_t run_nbody_step(
    float4* positions_read, float3* velocities_read, 
    float4* positions_write, float3* velocities_write,
    float dt, int num_bodies, float softening);