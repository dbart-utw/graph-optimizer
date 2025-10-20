#ifndef DATASTRUCTURES_HPP
#define DATASTRUCTURES_HPP

#include <cstddef>

template<typename T>
class CArray {
public:
    T* data;
    size_t size;

    CArray() {
        this->data = nullptr;
        this->size = 0;
    }

    virtual void init(size_t size) {
        this->data = new T[size];
        this->size = size;
    }

    virtual void free() {
        delete[] this->data;
        this->data = nullptr;
        this->size = 0;
    }

    CArray(size_t size) {
        this->init(size);
    }
};

template<typename T>
class CMatrix {
public:
    T* data;
    size_t size_m, size_n;

    CMatrix() {
        this->data = nullptr;
        this->size_m = 0;
        this->size_n = 0;
    }

    virtual void init(size_t size_m, size_t size_n) {
        this->data = new T[size_m * size_n];
        for (size_t i = 0; i < size_m * size_n; i++) {
            this->data[i] = 0;
        }
        this->size_m = size_m;
        this->size_n = size_n;
    }

    virtual void free() {
        delete[] this->data;
        this->data = nullptr;
        this->size_m = 0;
        this->size_n = 0;
    }

    CMatrix(size_t size_m, size_t size_n) {
        this->init(size_m, size_n);
    }
};

#endif
