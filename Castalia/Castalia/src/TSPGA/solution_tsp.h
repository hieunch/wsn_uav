#ifndef solution_tsp_H
#define solution_tsp_H

#include <cstdio>
#include <list>
#include <vector>
#include "customer_tsp.h"
#include "route_tsp.h"
#include "problem_tsp.h"
#include "CastaliaModule.h"
#include <chrono>
#include <sstream>

class solution_tsp{
public:
	void print(const problem_tsp& input) const;
	void clear();
	void random(const problem_tsp& input);
	void fitness(const problem_tsp& input);
	static int cmp(const solution_tsp &solA, const solution_tsp &solB, const problem_tsp &input);
	static bool fdominate(const solution_tsp &solA, const solution_tsp &solB);
	static bool idominate(const solution_tsp &solA, const solution_tsp &solB);
	bool operator == (const solution_tsp &another) const;
	bool operator < (const solution_tsp &another) const;

	vector<vector<int>> toTrajectories(const problem_tsp &input);

	route_tsp route;
	double totalDistance, totalTimewarp, totalWaiting, maxDistance;
	int unbalancedCapacity, exceededCapacity;
	bool feasible;

	static CastaliaModule *module;
};

#endif