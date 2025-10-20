#ifndef GPU_DATASTRUCTURES_HPP
#define GPU_DATASTRUCTURES_HPP

#include <cstddef>

#include <cuda_runtime.h>

#include "datastructures.hpp"

template<typename T>
class GPU_CArray : public CArray<T> {
public:
    void init(size_t size) override {
        cudaMalloc((void **) &this->data, size * sizeof(T));
        this->size = size;
    }

    void free() override {
        cudaFree(this->data);
        this->data = nullptr;
        this->size = 0;
    }
};

template<typename T>
class GPU_CMatrix : public CMatrix<T> {
public:
    void init(size_t size_m, size_t size_n) override {
        cudaMalloc((void **) &this->data, size_m * size_n * sizeof(T));
        this->size_m = size_m;
        this->size_n = size_n;
    }

    void free() override {
        cudaFree(this->data);
        this->data = nullptr;
        this->size_m = 0;
        this->size_n = 0;
    }
};

#endif