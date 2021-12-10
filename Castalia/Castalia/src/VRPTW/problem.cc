#include "problem.h"
#include "GlobalLocationService.h"
#include <fstream>


bool problem::load(int sinkId, std::vector<int> landmarks, int numUAVs, double maxDelay){
	
	allCustomer.reserve(landmarks.size() + 1);    // with depot
	maxRoutes = numUAVs;

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
		customer newCustomer(index, x, y, d, s, e, u);
		allCustomer.push_back(newCustomer);
		index++;
	}	
	
	calDistances();

	// double avgCusPerVehicle = capacity / (totalDemand / inputNum);
	// shortHorizon = (calMinRoute() > avgCusPerVehicle);

	return true;
}

bool problem::load2(std::string filename){
	
	std::vector<std::pair<double,double>> points;
	std::fstream myfile(filename, std::ios_base::in);

    double x,y;
    while (myfile >> x >> y)
    {
        points.push_back({x,y});
    }

	
	allCustomer.reserve(points.size());    // with depot
	maxRoutes = 1;
	
	for (int i=0; i<points.size(); i++) {
		realIds.push_back(i);
	}

	int totalDemand = 0;
	int index = 0;
	for(int l : realIds){

		auto p = points[l];
		double x = p.first;
		double y = p.second;
		int d = 0;
		int s = 0;
		int e = ceil(10000);
		int u = 0;

		// totalDemand += d;
		customer newCustomer(index, x, y, d, s, e, u);
		allCustomer.push_back(newCustomer);
		index++;
	}	
	
	calDistances();

	// double avgCusPerVehicle = capacity / (totalDemand / inputNum);
	// shortHorizon = (calMinRoute() > avgCusPerVehicle);

	return true;
}

int problem::getCapacity() const {
	return capacity;
}

int problem::getNumCusto() const {
	return allCustomer.size() - 1;    // ignore depot 
}

double problem::getDistance(int id1, int id2) const {
	return distance[id1][id2];
}

const customer& problem::operator [] (int id) const {
	return allCustomer[id];
}

// minimum # of route = ceil(total demand / vehicle's capacity);
int problem::calMinRoute() const {
	int totalDemand = getCapacity() - 1;    // ceiling
	for(int i = 1; i <= getNumCusto(); ++i) totalDemand += allCustomer[i].demand;
	return totalDemand / getCapacity();
}

void problem::calDistances(){
	distance.resize( allCustomer.size() );

	for(unsigned int i = 0; i < allCustomer.size(); ++i){
		distance[i].resize( allCustomer.size() );
	}

	for(unsigned int x = 0;   x < allCustomer.size(); ++x){
	for(unsigned int y = x+1; y < allCustomer.size(); ++y){
		double ans = hypot(allCustomer[x].x_pos - allCustomer[y].x_pos,
						   allCustomer[x].y_pos - allCustomer[y].y_pos);
		distance[x][y] = distance[y][x] = ans;
	}}
}


// void problem::calDistances(){
// 	distance.resize( allCustomer.size() );

// 	for(unsigned int i = 0; i < allCustomer.size(); ++i){
// 		distance[i].resize( allCustomer.size() );
// 	}

// 	// for(unsigned int x = 0;   x < allCustomer.size(); ++x){
// 	// for(unsigned int y = 0; y < allCustomer.size(); ++y){
// 	// 	distance[x][y] = DBL_MAX;
// 	// }}

// 	vector<double> neighborRange(allCustomer.size(), DBL_MAX);
// 	for(unsigned int x = 0;   x < allCustomer.size(); ++x){
// 	for(unsigned int y = 0; y < allCustomer.size(); ++y){
// 		if (y == x) continue;
// 		double ans = hypot(allCustomer[x].x_pos - allCustomer[y].x_pos,
// 						   allCustomer[x].y_pos - allCustomer[y].y_pos);
// 		if (ans < neighborRange[x]) neighborRange[x] = ans;
// 	}}

// 	for(unsigned int x = 0;   x < allCustomer.size(); ++x){
// 	for(unsigned int y = 0; y < allCustomer.size(); ++y){
// 		if (y == x) continue;
// 		double ans = hypot(allCustomer[x].x_pos - allCustomer[y].x_pos,
// 						   allCustomer[x].y_pos - allCustomer[y].y_pos);
// 		if (ans < 2*neighborRange[x]) distance[x][y] = distance[y][x] = ans;
// 		else distance[x][y] = distance[y][x] = 16*ans;
// 	}}
// }
