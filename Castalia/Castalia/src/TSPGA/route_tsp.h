#ifndef route_tsp_H
#define route_tsp_H

#include <cstdio>
#include <list>
#include <vector>
#include "customer_tsp.h"
#include "problem_tsp.h"
#include "CastaliaModule.h"
#include "GlobalLocationService.h"

class route_tsp{
public:
	void print(const problem_tsp& input) const;
	void clear();
	void fitness(const problem_tsp& input);
	static int cmp(const route_tsp& route_tspA, const route_tsp& route_tspB, const problem_tsp& input);
	bool hasCus(int cusID) const;

	std::vector<int> visits;
	double distance, timewarp, waiting;
	int load;
	bool feasible, modified;

	static CastaliaModule *module;
};

#endif