#include "CpuSubsetSolver.h"
#include <vector>
#include <algorithm>

bool NaiveSubsetSolver::solveRecursive(int sum, int* tab, int begin, int n){
	if(sum == 0)
		return true;
	if(sum < 0) return false;
	if(begin == n) return false;
	int currentElement = tab[begin];
	bool withThis = solveRecursive(sum-currentElement, tab, begin+1, n);
	if (withThis) return true;
	bool withoutThis = solveRecursive(sum, tab, begin+1, n);
	if(withoutThis) return true;
	return false;
}

bool NaiveSubsetSolver::solve(int sum, int* tab, int n){
	return solveRecursive(sum, tab, 0, n);
}


bool TwoListSubsetSolver::solve(int sum, int* tab, int n){
	int* firstTab = tab;
	int firstSize = n / 2;
	int* secondTab = tab + firstSize;
	int secondSize = n - firstSize;

	std::vector<int>* increasing = generateIncreasing(firstTab, firstSize);
	std::vector<int>* decreasing = generateDecreasing(secondTab, secondSize);

	bool found = search(increasing, decreasing, sum);
	delete increasing;
	delete decreasing;
	return found;
}

std::vector<int>* TwoListSubsetSolver::generateIncreasing(int* tab, int size){
	std::vector<int>* result = new std::vector<int>();
	std::vector<int>* tmp1 = new std::vector<int>();
	std::vector<int>* tmp2 = new std::vector<int>();
	result->push_back(0);
	result->push_back(*tab);

	for(int i = 1; i < size; i++){
		for(std::vector<int>::iterator it = result->begin(), en = result->end(); it != en; it++){
			tmp1->push_back((*it) + tab[i]);
		}
		mergeIncreasing(result, tmp1, tmp2);
		std::swap(result, tmp2);
		tmp1->clear();
		tmp2->clear();
	}
	delete tmp1;
	delete tmp2;
	return result;
}

void TwoListSubsetSolver::mergeIncreasing(std::vector<int>* a, std::vector<int>* b, std::vector<int>* result){
	std::vector<int>::iterator ita = a->begin(), itb = b->begin(), enda = a->end(), endb = b->end();
	while(ita != enda && itb != endb){
		if((*ita) > (*itb)){
			result->push_back(*(itb++));
		} else{
			result->push_back(*(ita++));
		}
	}
	while(ita != enda){
		result->push_back(*(ita++));
	}
	while(itb != endb){
		result->push_back(*(itb++));
	}
}

std::vector<int>* TwoListSubsetSolver::generateDecreasing(int* tab, int size){
	std::vector<int>* result = generateIncreasing(tab, size);
	std::reverse(result->begin(), result->end());
	return result;
}

bool TwoListSubsetSolver::search(std::vector<int>* increasing, std::vector<int>* decreasing, int sum){
	std::vector<int>::iterator inc = increasing->begin(), dec = decreasing->begin(), ince = increasing->end(), dece = decreasing->end();
	while(inc != ince && dec != dece){
		int value = *inc + *dec;
		if(value == sum) return true;
		if(value < sum) inc++;
		else dec++;
	}
	return false;
}
