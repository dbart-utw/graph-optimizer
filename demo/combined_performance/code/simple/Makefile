PYTHON          = /usr/bin/env python

CPP_COMPILER    = g++ -std=c++20
CPP_FLAGS       = -O3 -march=native -Wall -Wextra -fPIC
CPP_INCLUDES    = -I$(HOME)/.local/include -I../../../include
CPP_LIBS        = -llagraph -lgraphblas -lm -Wl,-rpath,$(HOME)/.local/lib64 -L$(HOME)/.local/lib64

HEADER  = $(notdir $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))).hpp
SOURCES = $(filter-out bench.cpp, $(wildcard *.c *.cpp))
OBJECTS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
ADDITIONAL_OBJECTS = ../../../bgo/bc/bc_lagr/bc_lagr.o ../../../bgo/find_max/find_max_gb/find_max_gb.o ../../../bgo/bfs/bfs_lagr/bfs_lagr.o

all: bench

bench.cpp: $(HEADER)
	$(PYTHON) ../../../autobench/gen_bench_code.py $@ $^ '$(CPP_INCLUDES)'

%.o: %.cpp
	$(CPP_COMPILER) $(CPP_FLAGS) -c $(CPP_INCLUDES) $< -o $@

bench: bench.cpp $(OBJECTS)
	$(CPP_COMPILER) $(CPP_FLAGS) $(CPP_INCLUDES) $(CPP_LIBS) $^ -o $@ $(ADDITIONAL_OBJECTS)

clean:
	rm -f bench bench.cpp *.o

.PHONY: all clean
