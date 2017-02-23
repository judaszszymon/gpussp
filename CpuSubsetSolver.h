#ifndef CPUSUBSETSOLVER_H_
#define CPUSUBSETSOLVER_H_

#include "subsetsum_interface.h"
#include <vector>


class NaiveSubsetSolver : SubsetSumInterface{
private:
	bool solveRecursive(int sum, int* tab, int begin, int n);

public:
	bool solve(int sum, int* tab, int n);
};

class TwoListSubsetSolver : SubsetSumInterface{
private:
	std::vector<int>* generateIncreasing(int* tab, int size);
	std::vector<int>* generateDecreasing(int* tab, int size);
	bool search(std::vector<int>* increasing, std::vector<int>* decreasing, int sum);
	void mergeIncreasing(std::vector<int>* a, std::vector<int>* b, std::vector<int>* result);


public:
	bool solve(int sum, int* tab, int n);
};

#endif /* CPUSUBSETSOLVER_H_ */
