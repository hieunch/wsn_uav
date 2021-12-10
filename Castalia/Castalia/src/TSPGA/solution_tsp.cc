#include <algorithm>

#include "solution_tsp.h"

using namespace std;

CastaliaModule* solution_tsp::module;

void solution_tsp::print(const problem_tsp& input) const {
	for (int jj=0; jj<10; jj++) module->trace() << "[ route, distance = " << maxDistance << ", timewarp = " << totalTimewarp << ", unbalance = " << unbalancedCapacity << " " << feasible << "]";
	
	int count = 0;
	route.print(input);
}

void solution_tsp::clear(){
	// route = route_tsp();
	route.clear();
	totalDistance = totalTimewarp = totalWaiting = unbalancedCapacity = exceededCapacity = maxDistance = 0;
}

void solution_tsp::random(const problem_tsp& input){
	// clear();

	vector<int> newroute;

	// insert all customer_tsps into route randomly
	for(int id = 1; id <= input.getNumCusto(); ++id){
		newroute.push_back(id);
	}
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine e(seed);
	std::shuffle(std::begin(newroute), std::end(newroute), e);

	route.visits = vector<int>(newroute.begin(), newroute.end());
	route.modified = true;

	fitness(input);

	// module->trace() << "fitness " << route.size() << " " << maxDistance << " " << totalDistance;
}


void solution_tsp::fitness(const problem_tsp& input){
	totalDistance = totalTimewarp = totalWaiting = unbalancedCapacity = exceededCapacity = maxDistance = 0;
	feasible = true;

	route.fitness(input);
	totalDistance += route.distance;
	maxDistance = max(maxDistance, route.distance);
	// for (int jj=0; jj<10; jj++) module->trace() << "maxDistance " << maxDistance << " distance " << route.distance;
	// stringstream ss;
	// for(int i : route.visits) {
	// 	ss << input.realIds[i] << " ";
	// }
	// module->trace() << ss.str();
	if (route.visits.size() != input.getNumCusto()) {
		module->trace() << "visits size " << route.visits.size() << " num custo " << input.getNumCusto();
		stringstream ss;
		for(int i : route.visits) {
			ss << input.realIds[i] << " ";
		}
		module->trace() << ss.str();
	}
	totalTimewarp += route.timewarp;
	totalWaiting += route.waiting;
	unbalancedCapacity += (route.load > input.getCapacity()) ? 
		(route.load - input.getCapacity()) : (input.getCapacity() - route.load);
	if(route.load > input.getCapacity() ) exceededCapacity += (route.load - input.getCapacity());
	if(route.feasible == false) feasible = false;
}

int solution_tsp::cmp(const solution_tsp &solA, const solution_tsp &solB, const problem_tsp &input){
	if(solA.feasible != solB.feasible){
		if(solA.feasible) return -1;
		else return 1;
	}else if(solA.feasible){
		if(solA.maxDistance < solB.maxDistance) return -1;
		else return 1;
	}else{
		if(solA.maxDistance < solB.maxDistance) return -1;
		else return 1;
	}
}

bool solution_tsp::fdominate(const solution_tsp &solA, const solution_tsp &solB){
	return (solA.maxDistance < solB.maxDistance);
}

bool solution_tsp::idominate(const solution_tsp &solA, const solution_tsp &solB){
	return (solA.maxDistance < solB.maxDistance);
}

bool solution_tsp::operator == (const solution_tsp &another) const {
	// return (fabs(maxDistance - another.maxDistance) < 0.01);
	// return (route.visits == another.route.visits);
	return false;
}

bool solution_tsp::operator < (const solution_tsp &another) const {
	return (maxDistance < another.maxDistance);
}

vector<vector<int>> solution_tsp::toTrajectories(const problem_tsp &input) {
	vector<vector<int>> trajectories;

	std::vector<int> path;
	path.push_back(input.realIds[0]);
	for(int i : route.visits){
		path.push_back(input.realIds[i]);
	}
	path.push_back(input.realIds[0]);
	trajectories.push_back(path);
	
	return trajectories;
}