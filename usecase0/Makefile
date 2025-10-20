INCDIR      = /usr/local/include
LIBDIR      = /usr/local/lib

include make.def

SOURCES		= ../bgo/*.cpp main.cpp
CUDASOURCES = ../bgo/*.cu
OBJECTS	=	$(SOURCES:.cpp=.o) $(CUDASOURCES:.cu=.o)
TARGETS	=	main

all		: $(TARGETS)

../bgo/%.o	: ../bgo/%.cpp $(HEADERS)
		$(CXX) $(CXXFLAGS) -c $< -o $@

../bgo/%.o	: ../bgo/%.cu $(HEADERS)
		nvcc $(INCLUDE) -c $< -o $@

%.s 		: %.cpp $(HEADERS)
		$(CXX) -S $(CXXFLAGS) $<

%.o  		: %.cpp $(HEADERS)
		$(CXX) -c $(CXXFLAGS) $<

%  		: %.o
		$(CXX) $^ -o $@ $(LIB)

main: main.o ../bgo/find_max.o ../bgo/find_path.o ../bgo/bfs_gpu.o
	$(CXX) $^ -o $@ $(LIB)

clean		:
		/bin/rm -rf a.out $(TARGETS) $(OBJECTS) *~

.SECONDARY	: $(OBJECTS)

-include $(SOURCES:%.cpp=%.d)
