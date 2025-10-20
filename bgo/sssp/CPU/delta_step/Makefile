PYTHON          = python3

CPP_COMPILER    = g++
CPP_STD         = -std=c++20
CPP_FLAGS       = -O3 -march=native -Wall -Wextra -fPIC -fopenmp
CPP_INCLUDES    = -I../../../../include -I../../../../include/gap
CPP_LIBDIR      = -L../../../../lib
CPP_LIBS        = -lCPP_Joules -lgraphblas -llagraph -lm
CPP_RPATH       = -Wl,-rpath,../../../../lib

HEADER  = $(notdir $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))).hpp
SOURCES = $(filter-out bench.cpp, $(wildcard *.c *.cpp))
OBJECTS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

all: bench

bench.cpp: $(HEADER)
	$(PYTHON) ../../../../autobench/gen_bench_code.py $@ $^ '$(CPP_INCLUDES)'

%.o: %.cpp
	$(CPP_COMPILER) $(CPP_STD) $(CPP_FLAGS) $(CPP_INCLUDES) -c $< -o $@

bench: bench.cpp $(OBJECTS)
	$(CPP_COMPILER) $(CPP_STD) $(CPP_FLAGS) $(CPP_INCLUDES) $(CPP_LIBDIR) $^ $(CPP_LIBS) $(CPP_RPATH) -o $@

clean:
	rm -f bench bench.cpp *.o

.PHONY: all clean
