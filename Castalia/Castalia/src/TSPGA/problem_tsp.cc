#include "problem_tsp.h"
#include "GlobalLocationService.h"

CastaliaModule* problem_tsp::module;

bool problem_tsp::load(int sinkId, std::vector<int> landmarks, int numUAVs, double maxDelay){
	
	allcustomer_tsp.reserve(landmarks.size() + 1);    // with depot
	maxroute_tsps = numUAVs;

	realIds.push_back(sinkId);
	realIds.insert(realIds.end(), landmarks.begin(), landmarks.end());

	int totalDemand = 0;
	int index = 0;
	for(int l : realIds){

		Point p = GlobalLocationService::getLocation(l);
		double x = p.x_;
		double y = p.y_;
		int d = 0;
		int s = 0;
		int e = ceil(maxDelay);
		int u = 0;

		// totalDemand += d;
		customer_tsp newcustomer_tsp(index, x, y, d, s, e, u);
		allcustomer_tsp.push_back(newcustomer_tsp);
		index++;
	}	
	
	calDistances();

	// double avgCusPerVehicle = capacity / (totalDemand / inputNum);
	// shortHorizon = (calMinroute_tsp() > avgCusPerVehicle);

	return true;
}

int problem_tsp::getCapacity() const {
	return capacity;
}

int problem_tsp::getNumCusto() const {
	return allcustomer_tsp.size() - 1;    // ignore depot 
}

double problem_tsp::getDistance(int id1, int id2) const {
	return distance[id1][id2];
}

const customer_tsp& problem_tsp::operator [] (int id) const {
	return allcustomer_tsp[id];
}

// minimum # of route_tsp = ceil(total demand / vehicle's capacity);
int problem_tsp::calMinroute_tsp() const {
	int totalDemand = getCapacity() - 1;    // ceiling
	for(int i = 1; i <= getNumCusto(); ++i) totalDemand += allcustomer_tsp[i].demand;
	return totalDemand / getCapacity();
}

void problem_tsp::calDistances(){
	distance.resize( allcustomer_tsp.size() );

	for(unsigned int i = 0; i < allcustomer_tsp.size(); ++i){
		distance[i].resize( allcustomer_tsp.size() );
	}

	for(unsigned int x = 0;   x < allcustomer_tsp.size(); ++x){
	for(unsigned int y = x+1; y < allcustomer_tsp.size(); ++y){
		double ans = hypot(allcustomer_tsp[x].x_pos - allcustomer_tsp[y].x_pos,
						   allcustomer_tsp[x].y_pos - allcustomer_tsp[y].y_pos);
		distance[x][y] = distance[y][x] = ans;
	}}
}
