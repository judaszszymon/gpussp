#include "subsetsum.h"
#include <cuda.h>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include "vector_types.h"

#define MAX_THREADS (2048 * 4)
#define _(x) getError((x), __LINE__)
void getError(CUresult, int line);


SubsetSolver::SubsetSolver(){
	initializeCuda();
}

SubsetSolver::~SubsetSolver(){}

bool checkForTrue(CUdeviceptr deviceValue){
	int value;
	_(cuMemcpyDtoH(&value, deviceValue, sizeof(int)));
	return value;
}

bool SubsetSolver::solve(int sum, int* tab, int n){
	int firstSize = n / 2;
	int secondSize = n - firstSize;
	int* firstTab = tab;
	int* secondTab = tab + firstSize;

	CUdeviceptr d_firstTab, d_secondTab;
	CUdeviceptr listA, listB;
	int sizeA = 1 << firstSize, sizeB = 1 << secondSize;

	_(cuMemAlloc(&d_firstTab, sizeof(int) * firstSize));
	_(cuMemcpyHtoD(d_firstTab,(void*) firstTab, sizeof(int) * firstSize));
	listA = generateIncreasing(d_firstTab, firstSize);
	_(cuMemFree(d_firstTab));

	_(cuMemAlloc(&d_secondTab, sizeof(int) * secondSize));
	_(cuMemcpyHtoD(d_secondTab,(void*) (firstTab + firstSize), sizeof(int) * secondSize));
	listB = generateDecreasing(d_secondTab, secondSize);
	_(cuMemFree(d_secondTab));

	CUdeviceptr d_found, d_pickedBlocks, d_pickedBlocksCounter;
	prune(listA, sizeA, listB, sizeB, sum, &d_found, &d_pickedBlocks, &d_pickedBlocksCounter);
	bool found = checkForTrue(d_found);
	if (found) return true;
	search(listA, sizeA, listB, sizeB, sum, d_pickedBlocks, d_pickedBlocksCounter, d_found);
	found = checkForTrue(d_found);

	_(cuMemFree(d_found));
	_(cuMemFree(d_pickedBlocks));
	_(cuMemFree(d_pickedBlocksCounter));
	_(cuMemFree(listA));
	_(cuMemFree(listB));

	return found;

}

// TODO modify Agnieszka's code below
void SubsetSolver::initializeCuda(){
	_(cuInit(0));
	_(cuDeviceGet(&cuda_device, 0));
	_(cuCtxCreate(&cuda_context, CU_CTX_SCHED_SPIN | CU_CTX_MAP_HOST, cuda_device));
	_(cuModuleLoad(&cuda_kernels, "kernels.ptx"));

	_(cuModuleGetFunction(&cuda_add, cuda_kernels, "cuAdd"));
	_(cuModuleGetFunction(&cuda_partition, cuda_kernels, "cuPartition"));
	_(cuModuleGetFunction(&cuda_mergeInc, cuda_kernels, "cuMergeIncreasing"));
	_(cuModuleGetFunction(&cuda_reverse, cuda_kernels, "cuReverse"));
	_(cuModuleGetFunction(&cuda_prune, cuda_kernels, "cuPrune"));
	_(cuModuleGetFunction(&cuda_search, cuda_kernels, "cuSearch"));
}

CUdeviceptr SubsetSolver::generateIncreasing(CUdeviceptr sourceTab, int sourceSize){
	CUdeviceptr list_A, temp;
	_(cuMemAlloc(&list_A, sizeof(int) * (1 << sourceSize)));
	_(cuMemAlloc(&temp, sizeof(int) * (1 << sourceSize)));

	_(cuMemsetD32(list_A, 0, (1 << sourceSize)));
	_(cuMemcpy(list_A + sizeof(int), sourceTab, sizeof(int)));

	int sizeOfList = 2, i, threads = 1;
	void* addArgs[] = {&list_A, &sourceTab, &i, &sizeOfList};
	for(i = 1; i < sourceSize; i++){

		_(cuLaunchKernel(cuda_add, (sizeOfList+1023) / 1024, 1, 1, 1024, 1, 1, 0, 0, addArgs, 0));
		mergeIncreasing(list_A, temp, sizeOfList);
		sizeOfList *= 2;
		std::swap(list_A, temp);

	}
	_(cuMemFree(temp));
	return list_A;
}

CUdeviceptr SubsetSolver::generateDecreasing(CUdeviceptr sourceTab, int sourceSize){

	CUdeviceptr list_B = generateIncreasing(sourceTab, sourceSize), tmp;
	int size = 1 << sourceSize;
	void* reverseArgs[] = {&list_B, &size};
	_(cuLaunchKernel(cuda_reverse, (sourceSize+1023) / 1024, 1, 1, 1024, 1, 1, 0, 0, reverseArgs, 0));
	return list_B;
}

void SubsetSolver::mergeIncreasing(CUdeviceptr lists, CUdeviceptr result, int size){
	CUdeviceptr H;
	int threads = size < MAX_THREADS ? size : MAX_THREADS;
	_(cuMemAlloc(&H, sizeof(int4) * threads * 2));
	_(cuMemsetD8(H, (char) 0, 2*threads));
	int4 preInit[2];
	preInit[1].x = 0;
	preInit[1].y = size;
	preInit[1].z = 0;
	preInit[1].w = size;
	_(cuMemcpyHtoD(H, &(preInit[0]), 2 * sizeof(int4)));

	int i, j;
	void* partitionArgs[] = {&j, &lists, &H, &size};

	for(i = 1, j = 1; i < threads; i *= 2, j++){
		_(cuLaunchKernel(cuda_partition, (threads+31) / 32, 1, 1, 32, 1, 1, 0, 0, partitionArgs, 0));
		_(cuCtxSynchronize());
	}
	void* mergeArgs[] = {&lists, &H, &size, &threads, &result};
	_(cuLaunchKernel(cuda_mergeInc, (threads+31) / 32, 1, 1, 32, 1, 1, 0, 0, mergeArgs, 0));
	_(cuCtxSynchronize());

	_(cuMemFree(H));
}

void SubsetSolver::prune(CUdeviceptr listA, int sizeA, CUdeviceptr listB, int sizeB, int M, CUdeviceptr* d_found, CUdeviceptr* d_pickedBlocks, CUdeviceptr* d_pickedBlocksCounter){
	int threads = 1024*4;
	threads = std::min(threads, std::min(sizeA, sizeB));
	_(cuMemAlloc(d_found, sizeof(int)));
	_(cuMemsetD32(*d_found, 0, 1));
	_(cuMemAlloc(d_pickedBlocks, sizeof(int2) * threads * 2));
	_(cuMemsetD8(*d_pickedBlocks, 0, sizeof(int2) * threads * 2));
	_(cuMemAlloc(d_pickedBlocksCounter, sizeof(int)));
	_(cuMemsetD32(*d_pickedBlocksCounter, 0, 1));
	_(cuCtxSynchronize());

//__global__ void cuPrune(int* listA, int sizeA, int* listB, int sizeB, int* found, int2* pickedBlocks, int* pickedBlocksCounter, int M)
	void* pruneArgs[] = {&listA, &sizeA, &listB, &sizeB, d_found, d_pickedBlocks, d_pickedBlocksCounter, &M};
	_(cuLaunchKernel(cuda_prune, (threads+31) / 32, 1, 1, 32, 1, 1, 0, 0, pruneArgs, 0));
	_(cuCtxSynchronize());

}


void SubsetSolver::search(CUdeviceptr listA, int sizeA, CUdeviceptr listB, int sizeB, int M, CUdeviceptr d_pickedBlocks, CUdeviceptr d_pickedBlocksCounter, CUdeviceptr d_found){
//__global__ void search(int* listA, int sizeA, int* listB, int sizeB, int2* pickedBlocks, int noPickedBlocks, int* found, int M)
	int threads = 1024*4;
	void* searchArgs[] = {&listA, &sizeA, &listB, &sizeB, &d_pickedBlocks, &d_pickedBlocksCounter, &d_found, &M};
	_(cuLaunchKernel(cuda_search, (threads+31) / 32, 1, 1, 32, 1, 1, 0, 0, searchArgs, 0));
	_(cuCtxSynchronize());

}


void SubsetSolver::printCuda(CUdeviceptr tab, int size){
	return;
	int* h_tab = new int[size];
	cuMemcpyDtoH(h_tab, tab, sizeof(int) * size);
	for(int i = 0; i < size;i++)
		std::cout << h_tab[i] << " ";
	std::cout << "\n";
	delete [] h_tab;
}

void getError(CUresult result, int line){
	if(result == CUDA_SUCCESS)
		return;
	const char* desc;
	const char** d = &desc;
	if(cuGetErrorString(result, d) == CUDA_SUCCESS)
		printf("%s",desc);
	else
		printf("unknown");
	exit(1);

}
