#ifndef PROBLEM_H
#define PROBLEM_H

#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include "customer.h"

class problem{
public:
	bool load(int sinkId, std::vector<int> landmarks, int numUAVs, double maxDelay);
	bool load2(std::string filename);
	int getCapacity() const;
	int getNumCusto() const;
	double getDistance(int id1, int id2) const;
	const customer& operator [] (int id) const;
	int calMinRoute() const;

	int maxRoutes;
	std::vector<int> realIds;

private:
	void calDistances();

	std::vector<customer> allCustomer;
	std::vector< std::vector<double> > distance;
	int capacity;
	bool shortHorizon;
};

#endif
