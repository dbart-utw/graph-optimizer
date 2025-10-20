#ifndef FIND_MAX_GPU_H
#define FIND_MAX_GPU_H

#include "datastructures.hpp"

#define BLOCKSIZE 256
#define warpSize 32

int find_max_gpu(CArray<int> v, int *index);

#endif