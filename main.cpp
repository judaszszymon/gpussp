#include "subsetsum.h"
#include "subsetsum_interface.h"
#include "CpuSubsetSolver.h"
#include <sys/time.h>
#include <iostream>
#include <iomanip>

void start_clock();
void stop_clock();
timeval tv;

int main(){
	int sum, n;
	std::cin >> sum >> n;
	int* tab = new int[n];
	for(int i = 0; i < n; i++){
		int x;
		std::cin >> x;
		tab[i] = x;
	}

	SubsetSolver solver;
	start_clock();
	bool result = solver.solve(sum, tab, n);
	std::cout << result << "\n";
	stop_clock();

	return 0;
}

void start_clock(){
	gettimeofday(&tv, NULL);
}
void stop_clock(){
	timeval end, diff;
	gettimeofday(&end, NULL);
	timersub(&end, &tv, &diff);
	std::cout << diff.tv_sec << "." << std::setfill('0') << std::setw(6) << diff.tv_usec << "\n";
}
