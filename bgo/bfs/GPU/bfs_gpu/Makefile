PYTHON        = python3

CPP_COMPILER  = g++
CPP_STD       = -std=c++20
CPP_FLAGS     = -O3 -march=native -Wall -Wextra -fPIC -fopenmp
CPP_INCLUDES  = -I$(HOME)/.local/include -I../../../../include -I../../../../include/gap -I/usr/local/cuda/include
CPP_LIBDIR    = -L../../../..//lib -L/usr/local/cuda/targets/x86_64-linux/lib
CPP_LIBS      = -lCPP_Joules -lgraphblas -llagraph -lcuda -lcudart -lm
CPP_RPATH     = -Wl,-rpath,../../../../lib

CUDA_COMPILER = /usr/local/cuda/bin/nvcc

HEADER  = $(notdir $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))).hpp
SOURCES = $(filter-out bench.cpp, $(wildcard *.c *.cpp *.cu))
OBJECTS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.cu,%.o,$(SOURCES)))))

all: bench

bench.cpp: $(HEADER)
	$(PYTHON) ../../../../autobench/gen_bench_code.py $@ $^ '$(CPP_INCLUDES)' --template ../../../../autobench/misc/template_cuda.cpp

%.o: %.cu
	$(CUDA_COMPILER) $(CPP_INCLUDES) -c $< -o $@

%.o: %.cpp
	$(CPP_COMPILER) $(CPP_STD) $(CPP_FLAGS) $(CPP_INCLUDES) -c $< -o $@

bench: bench.cpp $(OBJECTS)
	$(CPP_COMPILER) $(CPP_STD) $(CPP_FLAGS) $(CPP_INCLUDES) $(CPP_LIBDIR) $^ $(CPP_LIBS) $(CPP_RPATH) -o $@

clean:
	rm -f bench bench.cpp *.o

print-path:
	echo $$PATH

.PHONY: all clean
