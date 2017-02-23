#ifndef SUBSETSUM_INTERFACE_H_
#define SUBSETSUM_INTERFACE_H_

class SubsetSumInterface{
public:
	bool virtual solve(int sum, int* tab, int n) = 0;
	virtual ~SubsetSumInterface() = 0;
};


#endif /* SUBSETSUM_INTERFACE_H_ */
