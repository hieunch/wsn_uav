#ifndef ROUTE_H
#define ROUTE_H

#include <iostream>
#include <cstdio>
#include <list>
#include <vector>
#include "customer.h"
#include "problem.h"

class route{
public:
	void print(const problem& input) const;
	void clear();
	void fitness(const problem& input);
	static int cmp(const route& routeA, const route& routeB, const problem& input);
	bool hasCus(int cusID) const;

	std::list<int> visits;
	double distance, timewarp, waiting;
	int load;
	bool feasible, modified;

};

#endif
