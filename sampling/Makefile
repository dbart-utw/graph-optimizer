# Compiler settings
CXX := g++
# NVCC := nvcc
NVCC := g++

# Flags
CXXFLAGS := -O2 -std=c++20 -fopenmp
NVCCFLAGS := -O2 -std=c++20 -fopenmp

# Include directories
INCLUDES := -I../include

# Source files
# CPP_SRCS := sampling.cpp pagerank/pagerank_gap.cpp
CPP_SRCS := sampling.cpp
# CU_SRCS := $(wildcard pagerank/*.cu)
MAIN_SRC := main.cpp  # compiled with nvcc

# Object files
CPP_OBJS := $(CPP_SRCS:.cpp=.o)
CU_OBJS := $(CU_SRCS:.cu=.o)
MAIN_OBJ := $(MAIN_SRC:.cpp=.o)

# Output executable
TARGET := main

# Default rule
all: $(TARGET)

# Final linking
$(TARGET): $(CPP_OBJS) $(CU_OBJS) $(MAIN_OBJ)
	$(NVCC) $(NVCCFLAGS) -o $@ $^

# Compile pure C++ files with g++
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile CUDA files with nvcc
%.o: %.cu
	$(NVCC) $(NVCCFLAGS) $(INCLUDES) -c $< -o $@

# Compile main.cpp with nvcc
$(MAIN_OBJ): $(MAIN_SRC)
	$(NVCC) $(NVCCFLAGS) $(INCLUDES) -c $< -o $@

# Clean rule
clean:
	rm -f $(CPP_OBJS) $(CU_OBJS) $(MAIN_OBJ) $(TARGET)