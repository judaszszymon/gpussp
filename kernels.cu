#include<cstdio>
#include "vector_types.h"

extern "C" {


__global__ void cuAdd(int* list, int* elements, int i, int listSize){
	int thid = blockIdx.x * blockDim.x + threadIdx.x;
	while(thid < listSize){
		int value = list[thid] + elements[i];
		list[thid+listSize] = value;
		thid += blockDim.x * gridDim.x;
	}
}

__device__ int2 findMedian(int* tabA, int a, int b, int* tabB, int c, int d);

__global__ void cuPartition(int j, int* prevList, int4* H, int size){
	int* newList = prevList+size;
	int tid = blockIdx.x * blockDim.x + threadIdx.x;
	int threadCounts = 1 << (j-1);
	while (tid < threadCounts){
		int medianId = tid + threadCounts;
		int a = H[medianId].x;
		int b = H[medianId].y;
		int c = H[medianId].z;
		int d = H[medianId].w;
		int2 ef = findMedian(prevList, a, b, newList, c, d);
		H[2*medianId].x = a;
		H[2*medianId].y = ef.x;
		H[2*medianId].z = c;
		H[2*medianId].w = ef.y;
		H[2*medianId + 1].x = ef.x;
		H[2*medianId + 1].y = b;
		H[2*medianId + 1].z = ef.y;
		H[2*medianId + 1].w = d;
		tid += blockDim.x * gridDim.x;
	}
}

__device__ void mergeInc(int* listA, int beginA, int endA, int* listB, int beginB, int endB, int* result);

__global__ void cuMergeIncreasing(int* lists, int4* H, int listSize, int threads, int* result){
	int* newList = lists + listSize;
	int tid = blockIdx.x * blockDim.x + threadIdx.x + 1;
	while(tid <= threads){
		int medianId = tid + threads - 1;
		int4 localFetch = H[medianId];
		int a = localFetch.x;
		int b = localFetch.y;
		int c = localFetch.z;
		int d = localFetch.w;
		mergeInc(lists, a, b, newList, c, d, result);
		tid += blockDim.x * gridDim.x;
	}
}

__global__ void cuPrune(int* listA, int sizeA, int* listB, int sizeB, int* found, int2* pickedBlocks, int* pickedBlocksCounter, int M){
	int tid = blockIdx.x * blockDim.x + threadIdx.x;
	int k = blockDim.x*gridDim.x;
	int chunkA = (sizeA + k - 1)/ (k);
	int chunkB = (sizeB + k - 1)/ (k);
	for(int j = 0; j < k ; j++){
		if(*found) return;
		int x = listA[tid * chunkA] + listB[(j+1) * chunkB - 1]; // mozemy wyskoczyc jesli chunkA lub ChunbB nie dzieli k
		int y = listA[(tid+1) * chunkA - 1] + listB[j * chunkB]; // mozemy wyskoczyc tez
		if (x == M || y == M) atomicExch(found, 1);
		else if(x < M && y > M){
			int pos = atomicAdd(pickedBlocksCounter, 1);
			pickedBlocks[pos].x = tid;
			pickedBlocks[pos].y = j;
		}
	}
}

__device__ bool searchSteep(int* listA, int chunkSizeA, int* listB, int chunkSizeB, int M){
	int a, b;
	a = b = 0;
	while(a < chunkSizeA && b < chunkSizeB){
		int value = listA[a] + listB[b];
		if(value == M) return true;
		if(value < M) a++;
		else b++;
	}
	return false;
}

__global__ void cuSearch(int* listA, int sizeA, int* listB, int sizeB, int2* pickedBlocks, int* noPickedBlocks, int* found, int M){
	int thid = blockIdx.x * blockDim.x + threadIdx.x;
	int k = blockDim.x*gridDim.x;
	int chunkA = (sizeA + k - 1)/ (k);
	int chunkB = (sizeB + k - 1)/ (k);
	while(thid < *noPickedBlocks){
		if(*found) return;
		int2 idsOfFragmentToCheck = pickedBlocks[thid];
		int* shiftedListA = listA + idsOfFragmentToCheck.x * chunkA;
		int* shiftedListB = listB + idsOfFragmentToCheck.y * chunkB;
		int _sizeA = thid != k-1 ? chunkA : sizeA % chunkA;
		int _sizeB = thid != k-1 ? chunkB : sizeB % chunkB;

		bool f = searchSteep(shiftedListA, _sizeA, shiftedListB, _sizeB, M);
		if(f) *found = true;

		thid += k;
	}
}

__global__ void cuReverse(int* tab, int size){
	int tid = blockIdx.x * blockDim.x + threadIdx.x;
	if(tid >= size/2)
		return;
	int tmp = tab[tid];
	tab[tid] = tab[size-tid-1];
	tab[size-tid-1] = tmp;
}

__device__ int binsearchInc(int* tab, int l, int r, int value){
	while(l < r){
		int m = (l + r) / 2;
		if(tab[m] >= value){
			r = m;
		} else{
			l = m+1;
		}
	}
	return l;
}


__device__ int2 findMedian(int* tabA, int a, int b, int* tabB, int c, int d){
	int aMiddle, bMiddle, otherBegin, otherEnd, otherValue;
	int* otherTab;
	if(b-a > d-c){
		aMiddle = (b + a) / 2;
		otherTab = tabB;
		otherBegin = c;
		otherEnd = d;
		otherValue = tabA[aMiddle];
		//bMiddle = binsearchInc(tabB, c, d, tabA[aMiddle]);
	} else{
		bMiddle = (c + d) / 2;
		otherTab = tabA;
		otherBegin = a;
		otherEnd = b;
		otherValue = tabB[bMiddle];
		//aMiddle = binsearchInc(tabA, a, b, tabB[bMiddle]);
	}
	int theOtherMiddle = binsearchInc(otherTab, otherBegin, otherEnd, otherValue);
	if(b-a > d-c){
		bMiddle = theOtherMiddle;
	} else{
		aMiddle = theOtherMiddle;
	}
	int2 result;
	result.x = aMiddle;
	result.y = bMiddle;
	return result;
}

__device__ inline void mergeInc(int* listA, int beginA, int endA, int* listB, int beginB, int endB, int* result){
	int position = beginA + beginB;
	while(beginA < endA && beginB < endB){
		if (listA[beginA] < listB[beginB]){
			result[position++] = listA[beginA++];
		} else{
			result[position++] = listB[beginB++];
		}
	}

	while(beginA < endA){
		result[position++] = listA[beginA++];
	}
	while(beginB < endB){
		result[position++] = listB[beginB++];
	}

}



}
