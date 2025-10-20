# Compiler and flags
CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++17
LDFLAGS = -lgraphblas -llagraph -lCPP_Joules -Wl,-rpath=/usr/local/lib

# Source files
SRC = UC0_benchmark.cpp \
      ../../bgo/find_max/find_max_gb/find_max_gb.cpp \
      ../../bgo/find_path/find_path.cpp

# Output binary
TARGET = run_benchmark

# Default rule
all: $(TARGET)

# Link rule
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)
