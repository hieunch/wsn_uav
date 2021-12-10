#ifndef EVOLUTION_H
#define EVOLUTION_H

#include <stdlib.h> 
#include "problem.h"
#include "solution.h"
#include <sstream>

using namespace std;

bool reduceRoute(solution &sol, const problem& input);
solution crossover(const solution &a, const solution &b, const problem &input);
void mutation(solution &sol, const problem& input, int run);
const solution& tournament(const list<solution> &population, const problem &input);

void ranking(const list<solution> &population, vector< list<solution> > *output, bool feasible);
void environmental(const vector< list<solution> > &frank, const vector< list<solution> > &irank,
	list<solution> *output, unsigned int maxSize);

#endif