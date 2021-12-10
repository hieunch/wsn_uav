#ifndef problem_tsp_H
#define problem_tsp_H

#include <stdio.h>
#include <math.h>
#include <vector>
#include "customer_tsp.h"
#include "CastaliaModule.h"

class problem_tsp{
public:
	bool load(int sinkId, std::vector<int> landmarks, int numUAVs, double maxDelay);
	int getCapacity() const;
	int getNumCusto() const;
	double getDistance(int id1, int id2) const;
	const customer_tsp& operator [] (int id) const;
	int calMinroute_tsp() const;

	static CastaliaModule* module;
	int maxroute_tsps;
	std::vector<int> realIds;

private:
	void calDistances();

	std::vector<customer_tsp> allcustomer_tsp;
	std::vector< std::vector<double> > distance;
	int capacity;
	bool shortHorizon;
};

#endif