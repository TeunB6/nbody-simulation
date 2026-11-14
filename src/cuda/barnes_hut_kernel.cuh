#pragma once
#include <nbody/octree.hpp>
#include <cuda_runtime.h>

cudaError_t barnes_hut_step(const float4* positionsRead, const float3* velocitiesRead,
                            float4* positionsWrite, float3* velocitiesWrite,
                            const OctreeNode* nodes, int node_count,
                            float dt, int n, float softening, float theta);

cudaError_t barnes_hut_energy(const float4* positions, const float3* velocities, double* energy,
                            const OctreeNode* nodes, int node_count,
                            int n, float softening, float theta);
