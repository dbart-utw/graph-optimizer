/*
 * GPU Breadth First Search implementation based on:
 *     https://github.com/kaletap/bfs-cuda-gpu/blob/master/src/gpu/simple/bfs_simple.cu
 *         by Przemysław Kaleta (https://github.com/kaletap)
 */


#include "gpu_datastructures.hpp"

#include "bfs_gpu.hpp"

#define cudaDeviceScheduleBlockingSync 0x04 
#define BLOCK_SIZE 32


__global__
void initDeviceArray(int *array, int size, int alt_index) {
    int thread_index = blockIdx.x * blockDim.x + threadIdx.x;

    if (thread_index < size) {
        if (thread_index == alt_index) {
            array[thread_index] = 0;
        } else {
            array[thread_index] = -1;
        }
    }
}

__global__
void stepBreadthFirstSearchGPU(
    int *level, int *parent,
    int *G, int num_nodes,
    int *cQueue, int *nQueue, int *cQueueSize, int *nQueueSize, int cLevel
) {
    const int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < *cQueueSize) {
        int u = cQueue[tid];

        for (int v = 0; v < num_nodes; v++) {
            if (G[u * num_nodes + v] > 0 && (level[v] == -1)) {  // Reachable and unvisited
                parent[v] = u;
                level[v] = cLevel + 1;

                int i = atomicAdd(nQueueSize, 1);
                nQueue[i] = v;
            }
        }
    }
}

extern "C"
int bfs_gpu(
    GPU_CMatrix<int> G, int source,
    GPU_CArray<int> *level, GPU_CArray<int> *parent
) {
    const int num_nodes = G.size_m;
    const int blocks = (num_nodes + BLOCK_SIZE - 1) / BLOCK_SIZE;

    int nQueueSize = 1;
    int cLevel = 0;

    level->init(num_nodes);
    parent->init(num_nodes);

    int *d_cQueue;
    int *d_nQueue;
    int *d_cQueueSize;
    int *d_nQueueSize;

    /* Allocate memory on the device. */
    cudaMalloc((void **) &d_cQueue, num_nodes * sizeof(int));
    cudaMalloc((void **) &d_nQueue, num_nodes * sizeof(int));
    cudaMalloc((void **) &d_cQueueSize, sizeof(int));
    cudaMalloc((void **) &d_nQueueSize, sizeof(int));

    /* Initialize the level and parent arrays. */
    initDeviceArray<<<blocks, BLOCK_SIZE>>>(level->data, num_nodes, source);
    initDeviceArray<<<blocks, BLOCK_SIZE>>>(parent->data, num_nodes, -1);

    /* Put the root node in the queue. */
    cudaMemcpy(d_nQueue, &source, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_nQueueSize, &nQueueSize, sizeof(int), cudaMemcpyHostToDevice);  // This fails?!
    cudaDeviceSynchronize();

    while (nQueueSize > 0) {
        /* Swap current and next queue. */
        int *tmp = d_cQueue;
        d_cQueue = d_nQueue;
        d_nQueue = tmp;
        tmp = d_cQueueSize;
        d_cQueueSize = d_nQueueSize;
        d_nQueueSize = tmp;

        /* Make sure the next queue is empty! */
        cudaMemset(d_nQueueSize, 0, sizeof(int));
        cudaDeviceSynchronize();

        stepBreadthFirstSearchGPU<<<blocks, BLOCK_SIZE>>>(
            level->data, parent->data,
            G.data, num_nodes,
            d_cQueue, d_nQueue, d_cQueueSize, d_nQueueSize, cLevel
        );
        cudaDeviceSynchronize();

        cudaMemcpy(&nQueueSize, d_nQueueSize, sizeof(int), cudaMemcpyDeviceToHost);
        cudaDeviceSynchronize();

        if (++cLevel > num_nodes) {
            break;
        }
    }

    cudaFree(d_cQueue);
    cudaFree(d_nQueue);
    cudaFree(d_cQueueSize);
    cudaFree(d_nQueueSize);

    return 0;
}
