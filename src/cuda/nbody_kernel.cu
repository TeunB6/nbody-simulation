#include "nbody_kernel.cuh"
#include <cuda_runtime.h>
#include <cuda_gl_interop.h> 

#define CUDA_CHECK(x) do { cudaError_t e = (x); if (e != cudaSuccess) \
    throw std::runtime_error(std::string(cudaGetErrorString(e)) + " at " + __FILE__ + ":" + std::to_string(__LINE__)); \
 } while (0)



__global__ void nbody_compute_kernel(
    float4* positionsRead, float3* velocitiesRead,
    float4* positionsWrite, float3* velocitiesWrite,
    float dt, int n, float softening)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx > n) return;

    float4 myPos = positionsRead[idx];
    float3 myVel = velocitiesRead[idx];

    float3 accel = {0.0, 0.0, 0.0};

    // compute for each other particle (later switch to Barnes-Hut for nlogn)
    for (size_t j = 0; j < n; j++)
    {
        float4 otherPos = positionsRead[j];
        
        // relative pos vector
        float3 r;
        r.x = otherPos.x - myPos.x;
        r.y = otherPos.y - myPos.y;
        r.z = otherPos.z - myPos.z;

        // force calc
        float sqDist = r.x * r.x + r.y * r.y + r.z * r.z + softening;
        float invDist = rsqrtf(sqDist);
        float invDistCube = invDist * invDist * invDist;

        float force = otherPos.w * invDistCube;

        accel.x += force * r.x;
        accel.y += force * r.y;
        accel.z += force * r.z;
    }

    // update
    myVel.x += dt * accel.x;
    myVel.y += dt * accel.y;
    myVel.z += dt * accel.z;

    myPos.x += dt * myVel.x;
    myPos.y += dt * myVel.y;
    myPos.z += dt * myVel.z;

    velocitiesWrite[idx] = myVel;
    positionsWrite[idx] = myPos;  
}

cudaError_t run_nbody_step(
    float4* positionsRead, float3* velocitiesRead, 
    float4* positionsWrite, float3* velocitiesWrite,
    float dt, int num_bodies, float softening)
{
    // assume arrays alr allocated somehow
    int threadsPerBlock = 256;
    int blocks = (num_bodies + threadsPerBlock - 1) / threadsPerBlock;

    dim3 Dg(blocks,1,1);
    dim3 Db(threadsPerBlock,1,1);  
    nbody_compute_kernel<<<Dg,Db>>>(positionsRead, velocitiesRead,
         positionsWrite, velocitiesWrite, dt, num_bodies, softening);
    cudaError_t err = cudaGetLastError();
    return err;
}