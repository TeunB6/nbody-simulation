#include "barnes_hut_kernel.cuh"

#define BH_BLOCK 128
#define BH_STACK 64

__global__ void barnes_hut_step_kernel(const float4* positionsRead, const float3* velocitiesRead,
                            float4* positionsWrite, float3* velocitiesWrite,
                            const OctreeNode* nodes, int node_count,
                            float dt, int n, float softening, float theta)
{
    __shared__ int stack[BH_STACK][BH_BLOCK];
    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + tid;

    if (idx >= n) return;

    float4 myPos = positionsRead[idx];
    float3 myVel = velocitiesRead[idx];

    float3 accel = {0.0, 0.0, 0.0};
    float theta2 = theta * theta;
    float softening2 = softening * softening;

    int stackPtr = 0;
    stack[stackPtr++][tid] = 0; // start with the root node (index 0)
    while(stackPtr > 0)
    {
        
        const OctreeNode node = nodes[stack[--stackPtr][tid]];

        float3 r = {node.comMass.x - myPos.x,
                    node.comMass.y - myPos.y,
                    node.comMass.z - myPos.z};
        float dist2 = r.x * r.x + r.y * r.y + r.z * r.z;
        float size = node.halfWidth * 2.0f;

        bool contains = fabsf(myPos.x - node.center.x) <= node.halfWidth &&
            fabsf(myPos.y - node.center.y) <= node.halfWidth &&
            fabsf(myPos.z - node.center.z) <= node.halfWidth;

        bool acceptNode = node.isLeaf || (!contains && size * size / dist2 < theta2);
        if(!acceptNode) {
            // continue traversing
            for (int i = 0; i < 8; ++i) {
                int childIdx = node.child[i];
                if (childIdx <= -1) continue; // no child
                if (stackPtr < BH_STACK) {
                    stack[stackPtr++][tid] = childIdx;
                }
            }
            continue; // nodes that would overflow are skipped (can fix later if it seems to happen often)
        } 

        if (node.isLeaf && node.body == idx) continue; // skip

        // force calc
        float invDist = rsqrtf(dist2 + softening2);
        float invDistCube = invDist * invDist * invDist;

        float force = node.comMass.w * invDistCube;

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


cudaError_t barnes_hut_step(const float4* positionsRead, const float3* velocitiesRead,
                            float4* positionsWrite, float3* velocitiesWrite,
                            const OctreeNode* nodes, int node_count,
                            float dt, int n, float softening, float theta) {
    // assume arrays alr allocated somehow
    if (n <= 0) return cudaSuccess; // a 0-block grid is an invalid launch configuration
    int threadsPerBlock = BH_BLOCK;
    int blocks = (n + threadsPerBlock - 1) / threadsPerBlock;

    dim3 Dg(blocks,1,1);
    dim3 Db(threadsPerBlock,1,1);
    barnes_hut_step_kernel<<<Dg,Db>>>(positionsRead, velocitiesRead,
         positionsWrite, velocitiesWrite, nodes, node_count, dt, n, softening, theta);
    cudaError_t err = cudaGetLastError();
    return err;
}

__global__ void barnes_hut_energy_kernel(const float4* positionsRead, const float3* velocitiesRead, double* out,
                            const OctreeNode* nodes, int node_count, int n, float softening, float theta)
{
    __shared__ int stack[BH_STACK][BH_BLOCK];
    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + tid;
    double ke = 0.0, pe = 0.0;


    if (idx < n) {
        
        float4 myPos = positionsRead[idx];
        float3 myVel = velocitiesRead[idx];
        ke = 0.5 * myPos.w * (myVel.x * myVel.x + myVel.y * myVel.y + myVel.z * myVel.z);

        float theta2 = theta * theta;
        float softening2 = softening * softening;

        int stackPtr = 0;
        stack[stackPtr++][tid] = 0; // start with the root node (index 0)
        while(stackPtr > 0)
        {
            
            const OctreeNode node = nodes[stack[--stackPtr][tid]];

            float3 r = {node.comMass.x - myPos.x,
                        node.comMass.y - myPos.y,
                        node.comMass.z - myPos.z};
            float dist2 = r.x * r.x + r.y * r.y + r.z * r.z;
            float size = node.halfWidth * 2.0f;

            bool contains = fabsf(myPos.x - node.center.x) <= node.halfWidth &&
                fabsf(myPos.y - node.center.y) <= node.halfWidth &&
                fabsf(myPos.z - node.center.z) <= node.halfWidth;

            bool acceptNode = node.isLeaf || (!contains && size * size / dist2 < theta2);
            if(!acceptNode) {
                // continue traversing
                for (int i = 0; i < 8; ++i) {
                    int childIdx = node.child[i];
                    if (childIdx <= -1) continue; // no child
                    if (stackPtr < BH_STACK) {
                        stack[stackPtr++][tid] = childIdx;
                    }
                }
                continue; // nodes that would overflow are skipped (can fix later if it seems to happen often)
            } 

            if (node.isLeaf && node.body == idx) continue; // skip self (buckets have body == -1)

            float dist = sqrtf(dist2 + softening2);
            pe -= 0.5 * (myPos.w * node.comMass.w / dist);
        }
    } 
    
    unsigned int mask = 0xffffffff;
    for (int offset = warpSize / 2; offset > 0; offset /= 2) {
        pe += __shfl_down_sync(mask, pe, offset);
        ke += __shfl_down_sync(mask, ke, offset);
    }

    if ((threadIdx.x & 31) == 0) {
        atomicAdd(&out[0], ke);
        atomicAdd(&out[1], pe);
    } 
}

cudaError_t barnes_hut_energy(const float4* positions, const float3* velocities, double* energy,
                            const OctreeNode* nodes, int node_count,
                            int n, float softening, float theta) {
    if (n <= 0) return cudaSuccess; // a 0-block grid is an invalid launch configuration
    int threadsPerBlock = BH_BLOCK;
    int blocks = (n + threadsPerBlock - 1) / threadsPerBlock;

    cudaError_t err = cudaMemset(energy, 0, 2*sizeof(double));
    if (err != cudaSuccess) return err;

    dim3 Dg(blocks,1,1);
    dim3 Db(threadsPerBlock,1,1);  
    barnes_hut_energy_kernel<<<Dg,Db>>>(positions, velocities, energy, nodes, node_count, n, softening, theta);
    return cudaGetLastError();
}
