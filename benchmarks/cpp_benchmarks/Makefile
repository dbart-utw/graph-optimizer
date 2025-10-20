INCDIR      = ../../../include
LIBDIR      = ../../../lib

include make.def

CPP_SOURCES	= ../../bgo/*.cpp betweenness_centrality_benchmark.cpp sssp_benchmark.cpp bwc_dijkstra_benchmark.cpp find_path_benchmark.cpp
CU_SOURCES = ../../bgo/*.cu find_max_benchmark.cu
OBJECTS	=	$(CPP_SOURCES:.cpp=.o) $(CU_SOURCES:.cu=.o)
TARGETS	=	betweenness_centrality_benchmark sssp_benchmark bwc_dijkstra_benchmark find_max_benchmark find_path_benchmark

all		: $(TARGETS)

../../bgo/%.o	: ../../bgo/%.cpp $(HEADERS)
		$(CXX) $(CXXFLAGS) -c $< -o $@

../../bgo/%.o	: ../../bgo/%.cu $(HEADERS)
		$(NVCC) $(CUFLAGS) -c $< -o $@

%.s 		: %.cpp $(HEADERS)
		$(CXX) -S $(CXXFLAGS) $<

%.s 		: %.cu $(HEADERS)
		$(NVCC) -S $(CUFLAGS) $<

%.o  		: %.cpp $(HEADERS)
		$(CXX) -c $(CXXFLAGS) $<

%.o  		: %.cu $(HEADERS)
		$(NVCC) -c $(CUFLAGS) $<

%  		: %.o
		$(CXX) $^ -o $@ $(LIB)

betweenness_centrality_benchmark: betweenness_centrality_benchmark.o ../../bgo/betweenness_centrality.o
	$(CXX) $^ -o $@ $(LIB)

sssp_benchmark: sssp_benchmark.o ../../bgo/bfs.o ../../bgo/delta_step.o ../../bgo/dijkstra.o
	$(CXX) $^ -o $@ $(LIB)

find_path_benchmark: find_path_benchmark.o ../../bgo/find_path.o ../../bgo/bfs.o
	$(CXX) $^ -o $@ $(LIB)

find_max_benchmark: find_max_benchmark.o ../../bgo/find_max.o ../../bgo/find_max_gpu.o ../../bgo/find_max_parallel_cpu.o
	$(NVCC) $^ -o $@ $(CULIB)

clean		:
		/bin/rm -rf a.out $(TARGETS) $(OBJECTS) *~

.SECONDARY	: $(OBJECTS)

-include $(CPP_SOURCES:%.cpp=%.d)
-include $(CU_SOURCES:%.cu=%.d)
