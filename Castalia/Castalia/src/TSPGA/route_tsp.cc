#include <algorithm>

#include "route_tsp.h"

using namespace std;

CastaliaModule* route_tsp::module;

void route_tsp::print(const problem_tsp& input) const {
	std::vector<int> path;
	path.push_back(input.realIds[0]);
	for(vector<int>::const_iterator it = visits.begin(); it != visits.end(); ++it){
		path.push_back(input.realIds[*it]);
	}
	path.push_back(input.realIds[0]);
	for (int jj=0; jj<10; jj++) module->trace() << visits.size() << " customer_tsps, distance = " << distance << " : " << GlobalLocationService::calculatePathLength(path);
	
	for(vector<int>::const_iterator it = visits.begin(); it != visits.end(); ++it){
		module->trace() << input.realIds[*it];
	}

}

void route_tsp::clear(){
	visits.clear();
	distance = timewarp = waiting = load = 0;
	modified = true;
}

void route_tsp::fitness(const problem_tsp& input){
	if(!modified) return;

	vector<int> ids(visits.begin(), visits.end() );
	
	// reset all attributes
	distance = timewarp = waiting = load = 0;
	feasible = true;

	double arrival = input.getDistance(0, ids[0]);
	distance = arrival;
	for(unsigned int i = 0; i < ids.size(); ++i){
		load += input[ ids[i] ].demand;
		if( load > input.getCapacity() ) feasible = false;
	
		if(arrival > input[ ids[i] ].end){    // late arrival: time-wrap
			feasible = false;
			timewarp += arrival - input[ ids[i] ].end;
			arrival = input[ ids[i] ].end;
		}else if(arrival < input[ ids[i] ].start){    // early arrival: wait
			waiting += input[ ids[i] ].start - arrival;
			arrival = input[ ids[i] ].start;
		}

		// unload
		arrival += input[ ids[i] ].unload;

		// traveling
		if( i == ids.size() - 1 ){  // back to depot
			arrival += input.getDistance(ids[i], 0);
			distance += input.getDistance(ids[i], 0);
		}else{  // goto next customer_tsp
			arrival += input.getDistance(ids[i], ids[i+1]);
			distance += input.getDistance(ids[i], ids[i+1]);
		}
	}

	if(arrival > input[0].end){
		feasible = false;
		timewarp += arrival - input[0].end;
	}

	modified = false;
}

int route_tsp::cmp(const route_tsp& route_tspA, const route_tsp& route_tspB, const problem_tsp& input){
	return route_tspB.visits.size() - route_tspA.visits.size();
}

bool route_tsp::hasCus(int cusID) const {
	return ( find(visits.begin(), visits.end(), cusID) != visits.end() );
}