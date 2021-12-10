#ifndef evolution_tsp_H
#define evolution_tsp_H

#include "problem_tsp.h"
#include "solution_tsp.h"

using namespace std;

solution_tsp crossover(const solution_tsp &a, const solution_tsp &b, const problem_tsp &input);
void mutation(solution_tsp &sol, const problem_tsp& input);
const solution_tsp& tournament(const list<solution_tsp> &population, const problem_tsp &input);

void ranking(const list<solution_tsp> &population, vector< list<solution_tsp> > *output, bool feasible);
void environmental(const vector< list<solution_tsp> > &frank, const vector< list<solution_tsp> > &irank,
	list<solution_tsp> *output, unsigned int maxSize);

#endif