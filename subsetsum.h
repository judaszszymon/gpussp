#ifndef subsetsum_h
#define subsetsum_h
#include <cuda.h>
#include "subsetsum_interface.h"


class SubsetSolver : SubsetSumInterface {
private:
	CUdevice cuda_device;
	CUcontext cuda_context;
	CUmodule cuda_kernels;

	CUfunction cuda_add, cuda_partition, cuda_mergeInc, cuda_reverse, cuda_prune, cuda_search;

	void mergeIncreasing(CUdeviceptr lists, CUdeviceptr result, int size);
	void initializeCuda();
	void printCuda(CUdeviceptr tab, int size);
	void prune(CUdeviceptr listA, int sizeA, CUdeviceptr listB, int sizeB, int M, CUdeviceptr* d_found, CUdeviceptr* d_pickedBlocks, CUdeviceptr* d_pickedBlocksCounter);
	void search(CUdeviceptr listA, int sizeA, CUdeviceptr listB, int sizeB, int M, CUdeviceptr d_pickedBlocks, CUdeviceptr d_pickedBlocksCounter, CUdeviceptr d_found);

	// generate sums of all possible subsets and sort it
	CUdeviceptr generateIncreasing(CUdeviceptr sourceTab, int sourceSize);
	CUdeviceptr generateDecreasing(CUdeviceptr sourceTab, int sourceSize);

public:
	SubsetSolver();
	bool solve(int sum, int* tab, int n);
	 ~SubsetSolver();

};

#endif
