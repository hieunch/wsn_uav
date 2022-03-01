#include <vector>
#include <algorithm>
#include "evolution.h"
#include <cfloat>
#include <iostream>
#include <fstream>

using namespace std;

bool reduceRoute(solution &sol, const problem& input){
	// if(sol.routes.size() <= 1) return false;
	int numTry = 20;
	if(sol.routes.size() <= input.maxRoutes) return false;
	double before = sol.routes.size();
	
	// find route with highest (distance / # of customers).
	// list<route>::iterator minR;
	// double dis = 0;
	// for(list<route>::iterator it = sol.routes.begin(); it != sol.routes.end(); it++){
	// 	double p = it->distance;// / it->visits.size();
	// 	if(p > dis){
	// 		minR = it;
	// 		dis = p;
	// 	}
	// }

	int randomSelect = rand() % sol.routes.size();
	list<route>::iterator minR = sol.routes.begin();
	advance(minR, randomSelect);

	// remove this shortest route
	route shortest = (*minR);
	sol.routes.erase(minR);

	for(list<int>::iterator cus = shortest.visits.begin(); cus != shortest.visits.end();){
		int maxTry = 1;
		for (int j=0; j<maxTry; j++) {
			solution min = sol;

			double minMaxDistance = DBL_MAX;
			for(int tryCount = 0; tryCount < numTry; tryCount++){
				solution temp = sol;
				list<route>::iterator r = temp.routes.begin();
				advance(r, rand() % temp.routes.size() );
				list<int>::iterator ins = r->visits.begin();
				advance(ins, rand() % r->visits.size());
				r->visits.insert(ins, *cus);
				r->modified = true;
				temp.fitness(input);
				// if(temp.totalDistance < min.totalDistance){
				// if(temp.maxDistance < min.maxDistance){
				if(temp.maxDistance < minMaxDistance){
					minMaxDistance = temp.maxDistance;
					min = temp;
				}
			}

			// if(min.totalDistance < sol.totalDistance){
			if(min.maxDistance < sol.maxDistance || j == maxTry-1){
				cus = shortest.visits.erase(cus);
				shortest.modified = true;
				sol = min;
			}else{
				cus++;
			}
		}
	}

	// append a new route with customers can't be inserted.
	if( !shortest.visits.empty() ){
		sol.routes.push_front(shortest);
		ofstream minDistanceFile;
		minDistanceFile.open("MinDistance.txt", std::ios_base::app);
		minDistanceFile << "REDUCE ROUTE FAILED";
	}

	int numVisits = sol.validateNumVisits(input);
	if (numVisits != input.getNumCusto()) {
//		cout << "Wrong solution - reduceroute " << numVisits << " " << input.getNumCusto();
	}

	sol.fitness(input);

	return false;
	// return (sol.maxDistance < before);
	// return (sol.routes.size() < before);
}

solution crossover(const solution &pa, const solution &pb, const problem& input){
	solution offspring = pa;

	vector<route> bRoutes(pb.routes.begin(), pb.routes.end());
//	cout << "find best route";
	// find best route with smallest ratio (distance / # of customers).
	unsigned int maxR = bRoutes.size();
	double dis = 1e100;
	for(unsigned int i = 0; i < bRoutes.size(); ++i){
		if(bRoutes[i].feasible){
			double p = bRoutes[i].distance / bRoutes[i].visits.size();
			if(p < dis){
				maxR = i;
				dis = p;
			}
		}
	}
	
	if(maxR == bRoutes.size() ) maxR = rand() % bRoutes.size();

	if (pa.maxDistance != pb.maxDistance || pa.totalDistance != pb.totalDistance) {
		int maxR0 = maxR;
		bool check = true;
		while (check) {
			check = false;
			route selectedRoute = bRoutes[maxR];
			for (auto route : pa.routes) {
				if (selectedRoute.visits.size() == route.visits.size()) {
					list<int>::iterator it1 = selectedRoute.visits.begin();
					list<int>::iterator it2 = route.visits.begin();
					for (; it1 != selectedRoute.visits.end(); ++it1) {
						if (*it1 != *it2) break;
						++it2;
					}
					if (it1 == selectedRoute.visits.end()) check = true;
				}
			}
			if (check) maxR = (maxR+1) % bRoutes.size();
			if (maxR == maxR0) break;
		}
	}
//	cout << "remove best route's customer" << endl;
	// remove best route's customer
	for(list<int>::iterator cus = bRoutes[maxR].visits.begin(); cus != bRoutes[maxR].visits.end(); cus++){
	for(list<route>::iterator it = offspring.routes.begin(); it != offspring.routes.end(); ++it){
		list<int>::iterator todel = find(it->visits.begin(), it->visits.end(), *cus);
		if( todel != it->visits.end() ){
			it->visits.erase(todel);
			it->modified = true;
			break;
		}
	}}

//	cout << "remove empty route" << endl;
	// remove empty route
	for(list<route>::iterator it = offspring.routes.begin(); it != offspring.routes.end(); ){
		if(it->visits.size() == 0){
			it = offspring.routes.erase(it);
		}else{
			it++;
		}
	}

	offspring.routes.push_back(bRoutes[maxR]);

	int count = 0;
	while( reduceRoute(offspring, input) ) {
		if (++count > 50) cout << "reduceRoute" << endl;
	};

//	cout << "fitness" << endl;
	offspring.fitness(input);
	int numVisits = offspring.validateNumVisits(input);
	if (numVisits != input.getNumCusto()) {
//		cout << "Wrong solution - crossover " << numVisits << " " << input.getNumCusto() << endl;
	}
	return offspring;
}

void mutation(solution &sol, const problem& input, int run){
	int maxTry = 20;
	bool isChanged = false;
	int tryCount = 0;
	while(tryCount < maxTry) {//input.getNumCusto() ){
		solution test = sol;

		// randomly select two routes
		list<route>::iterator routeA = test.routes.begin();
		if (routeA->visits.size() == 1) {
			tryCount++;
			continue;
		}
		advance(routeA, rand() % test.routes.size() );
		list<route>::iterator routeB = test.routes.begin();
		advance(routeB, rand() % test.routes.size() );

		// # of feasible route BEFORE the reinsertion
		int beforeFeasibleCount = 0;
		if(routeA->feasible) beforeFeasibleCount++;
		if(routeB->feasible) beforeFeasibleCount++;

		// randomly select two positions
		list<int>::iterator cusA = routeA->visits.begin();
		advance(cusA, rand() % routeA->visits.size() );
		list<int>::iterator cusB = routeB->visits.begin();
		advance(cusB, rand() % routeB->visits.size() );

		routeB->visits.insert(cusB, *cusA);
		routeB->modified = true;
		routeA->visits.erase(cusA);
		routeA->modified = true;
		bool reduce = false;
		if( routeA->visits.empty() ){
			test.routes.erase(routeA);
			reduce = true;
		}

		test.fitness(input);

		int numVisits = test.validateNumVisits(input);
		if (numVisits != input.getNumCusto()) {
			cout << "Wrong solution - mutation " << numVisits << " " << input.getNumCusto() << endl;
		}
		
		// # of feasible route AFTER the reinsertion
		int afterFeasibleCount = 0;
		if(reduce || routeA->feasible) afterFeasibleCount++;
		if(routeB->feasible) afterFeasibleCount++;

		// if(afterFeasibleCount >= beforeFeasibleCount) sol = test;
		if (sol.maxDistance > test.maxDistance) {
			sol = test;
			sol.generation = run;
			isChanged = true;
		}
		else if (!isChanged && tryCount+1 == maxTry && rand() % 100 > 80) {
			sol = test;
			sol.generation = run;
		}

		tryCount++;
	}
}

// 2-tournament selection from population
const solution& tournament(const std::list<solution> &population, const problem &input){
//	cout << "tournament " << population.size() << endl;
	int selectA = rand() % population.size();
	int selectB = rand() % population.size();
	int maxRetry = 20;
	while (selectB == selectA && --maxRetry>0) selectB = rand() % population.size();

	list<solution>::const_iterator itA = population.begin();
	advance(itA, selectA);
	list<solution>::const_iterator itB = population.begin();
	advance(itB, selectB);

	int cmp = solution::cmp( (*itA), (*itB), input);
	if(cmp == 0) cmp = (rand() % 2) * 2 - 1;   // -1 or 1

	if(cmp < 0) return (*itA);
	else return (*itB);
}

// Use Deb's "Fast Nondominated Sorting" (2002)
// Ref.: "A fast and elitist multiobjective genetic algorithm: NSGA-II"
void ranking(const std::list<solution> &population, std::vector< std::list<solution> > *output, bool feasible){
	if( population.empty() ) return;
	vector<solution> solutions(population.begin(), population.end() );

	vector< list<int> > intOutput;
	intOutput.resize(solutions.size() + 2);  // start from 1, end with null Qi 
	output->resize(1);

	// record each solutions' dominated solution
	vector< list<int> > dominated;
	dominated.resize(solutions.size());

	// record # of solutions dominate this solution
	vector<int> counter;
	counter.resize(solutions.size());

	// record the rank of solutions
	vector<int> rank;
	rank.resize(solutions.size());

	
	// for each solution
	// vector<string> D_ss;
	// vector<string> C_ss;
	for(unsigned int p = 0; p < solutions.size(); p++){

		// stringstream d_ss;
		// d_ss << "dominate ";
		// stringstream c_ss;
		// c_ss << "count ";

		for(unsigned int q = 0; q < solutions.size(); q++){
			if( feasible ){
				if( solution::fdominate(solutions[p], solutions[q]) ){
					dominated[p].push_back(q);  // Add q to the set of solutions dominated by p
					// d_ss << q << " " << solutions[q].maxDistance << " ";
				}else if( solution::fdominate(solutions[q], solutions[p]) ){
					counter[p]++;
					// c_ss << q << " " << solutions[q].maxDistance << " ";
				}
			}else{
				if( solution::idominate(solutions[p], solutions[q]) ){
					dominated[p].push_back(q);  // Add q to the set of solutions dominated by p
					// d_ss << q << " " << solutions[q].maxDistance << " ";
				}else if( solution::idominate(solutions[q], solutions[p]) ){
					counter[p]++;
					// c_ss << q << " " << solutions[q].maxDistance << " ";
				}
			}
		}
		
		// D_ss.push_back(d_ss.str());
		// C_ss.push_back(c_ss.str());

		if(counter[p] == 0){  // p belongs to the first front
			rank[p] = 1;
			intOutput[1].push_back(p);
			(*output)[0].push_back(solutions[p]);
		}
	}

	int curRank = 1;
	while( intOutput[curRank].size() > 0 ){
		list<int> Qi;  // Used to store the members of the next front
		list<solution> Qs;

		for(list<int>::iterator p = intOutput[curRank].begin(); p != intOutput[curRank].end(); p++){
		for(list<int>::iterator q = dominated[(*p)].begin(); q != dominated[(*p)].end(); q++){
			counter[(*q)]--;
			if(counter[(*q)] == 0){  // q belongs to the next front
				rank[(*q)] = curRank + 1;
				Qi.push_back(*q);
				Qs.push_back(solutions[(*q)]);
			}
		}}

		curRank++;
		intOutput[curRank] = Qi;
		if(Qi.size() > 0) output->push_back(Qs);

		// if (curRank > 202) break;
	}

	// if ((curRank < 90) || (curRank > 202)) {
	// 	for(unsigned int p = 0; p < solutions.size(); p++){
	// 		solution::module->routing_trace() << p << " " << solutions[p].maxDistance;
	// 		solution::module->routing_trace() << D_ss[p];
	// 		solution::module->routing_trace() << C_ss[p];
	// 	}
	// }

	// remove duplicate solution in same rank
	for(unsigned int rank = 0; rank < output->size(); ++rank){
		(*output)[rank].sort();
		(*output)[rank].unique();
	}
}

void environmental(const vector< list<solution> > &frank, const vector< list<solution> > &irank, list<solution> *output, unsigned int maxSize){
	unsigned int curRank = 0;

	while(true){
		if(curRank < frank.size() && output->size() + frank[curRank].size() <= maxSize){
			output->insert(output->end(), frank[curRank].begin(), frank[curRank].end() );
		}else if(curRank < frank.size() ) break;

		if(curRank < irank.size() && output->size() + irank[curRank].size() <= maxSize){
			output->insert(output->end(), irank[curRank].begin(), irank[curRank].end() );
		}else if(curRank < irank.size() ) break;

		curRank++;
		if ((curRank >= frank.size()) && (curRank >= irank.size())) break;
		// if (curRank > 200) {
		// 	for(unsigned int rank = 0; rank < frank.size(); ++rank){
		// 		solution::module->routing_trace() << "frank " << rank << " size " << frank[rank].size();
		// 	}

		// 	for(unsigned int rank = 0; rank < irank.size(); ++rank){
		// 		solution::module->routing_trace() << "irank " << rank << " size " << irank[rank].size();
		// 	}

		// 	solution::module->routing_trace() << "curRank " << curRank << " size " << output->size();
		// 	exit(EXIT_FAILURE);
		// }
	}
	
	if(output->size() < maxSize && curRank < frank.size() ){
		vector<solution> nextRank(frank[curRank].begin(), frank[curRank].end() );

		while(output->size() < maxSize && !nextRank.empty()){
			unsigned int select = rand() % nextRank.size();
			output->push_back(nextRank[select]);
			nextRank.erase(nextRank.begin() + select);
		}
	}
	if(output->size() < maxSize && curRank < irank.size() ){
		vector<solution> nextRank(irank[curRank].begin(), irank[curRank].end() );

		while(output->size() < maxSize && !nextRank.empty()){
			unsigned int select = rand() % nextRank.size();
			output->push_back(nextRank[select]);
			nextRank.erase(nextRank.begin() + select);
		}
	}
}
