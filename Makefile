CUDA_INSTALL_PATH ?= /usr/local/cuda
VER = 
CXX := /usr/bin/g++$(VER)
CC := /usr/bin/gcc$(VER)
LINK := /usr/bin/g++$(VER) -fPIC
CCPATH := ./gcc$(VER)
NVCC  := $(CUDA_INSTALL_PATH)/bin/nvcc -ccbin $(CCPATH)

# Includes
INCLUDES = -I. -I$(CUDA_INSTALL_PATH)/include

# Libraries
LIB_CUDA := -L/usr/lib/nvidia-current -lcuda


# Options
NVCCOPTIONS = -arch sm_30 -ptx

# Common flags
COMMONFLAGS += $(INCLUDES)
NVCCFLAGS += $(COMMONFLAGS) $(NVCCOPTIONS)
CXXFLAGS += $(COMMONFLAGS)
CFLAGS += $(COMMONFLAGS)



CUDA_OBJS = kernels.ptx 
BAREOBJS = subsetsum.cpp.o CpuSubsetSolver.cpp.o subsetsum_interface.cpp.o
OBJS = subsetsum.cpp.o CpuSubsetSolver.cpp.o subsetsum_interface.cpp.o twolist_cpu.cpp.o brute_cpu.cpp.o main.cpp.o


all: gpu.x cpu.x twocpu.x

gpu.x: prepare $(OBJS) $(CUDA_OBJS)
	$(LINK) -o gpu.x $(BAREOBJS) main.cpp.o $(LIB_CUDA)
	
cpu.x: prepare $(OBJS) $(CUDA_OBJS)
	$(LINK) -o cpu.x $(BAREOBJS) brute_cpu.cpp.o $(LIB_CUDA)

twocpu.x: prepare $(OBJS) $(CUDA_OBJS)
	$(LINK) -o twocpu.x $(BAREOBJS) twolist_cpu.cpp.o $(LIB_CUDA)


.SUFFIXES:	.c	.cpp	.cu	.o	
%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.ptx: %.cu
	$(NVCC) $(NVCCFLAGS) $< -o $@

%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf gpu.x cpu.x twocpu.x *.o *.ptx

prepare:
	rm -rf $(CCPATH);\
	mkdir -p $(CCPATH);\
	ln -s $(CXX) $(CCPATH)/g++;\
	ln -s $(CC) $(CCPATH)/gcc;