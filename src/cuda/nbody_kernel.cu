#include "nbody_kernel.cuh"
#include <cuda_runtime.h>
#include <cuda_gl_interop.h> 

#define CUDA_CHECK(x) do { cudaError_t e = (x); if (e != cudaSuccess) \
    throw std::runtime_error(std::string(cudaGetErrorString(e)) + " at " + __FILE__ + ":" + std::to_string(__LINE__)); \
 } while (0)



__global__ void nbody_compute_kernel(
    float4* positions_read, float3* velocities_read,
    float4* positions_write, float3* velocities_write,
    float dt, int n, float softening)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx > n) return;

    float4 my_pos = positions_read[idx];
    float3 my_vel = velocities_read[idx];

    float3 accel = {0.0, 0.0, 0.0};

    // compute for each other particle (later switch to Barnes-Hut for nlogn)
    for (size_t j = 0; j < n; j++)
    {
        float4 other_pos = positions_read[j];
        
        // relative pos vector
        float3 r;
        r.x = other_pos.x - my_pos.x;
        r.y = other_pos.y - my_pos.y;
        r.z = other_pos.z - my_pos.z;

        // force calc
        float sq_dist = r.x * r.x + r.y * r.y + r.z * r.z + softening;
        float inv_dist = rsqrtf(sq_dist);
        float inv_dist_cube = inv_dist * inv_dist * inv_dist;

        float force = other_pos.w * inv_dist_cube;

        accel.x += force * r.x;
        accel.y += force * r.y;
        accel.z += force * r.z;
    }

    // update
    my_vel.x += dt * accel.x;
    my_vel.y += dt * accel.y;
    my_vel.z += dt * accel.z;

    my_pos.x += dt * my_vel.x;
    my_pos.y += dt * my_vel.y;
    my_pos.z += dt * my_vel.z;

    velocities_write[idx] = my_vel;
    positions_write[idx] = my_pos;  
}

cudaError_t run_nbody_step(
    float4* positions_read, float3* velocities_read, 
    float4* positions_write, float3* velocities_write,
    float dt, int num_bodies, float softening)
{
    // assume arrays alr allocated somehow
    int thread_per_block = 256;
    int blocks = (num_bodies + thread_per_block - 1) / thread_per_block;

    dim3 Dg(blocks,1,1);
    dim3 Db(thread_per_block,1,1);  
    nbody_compute_kernel<<<Dg,Db>>>(positions_read, velocities_read,
         positions_write, velocities_write, dt, num_bodies, softening);
    cudaError_t err = cudaGetLastError();
    return err;
}