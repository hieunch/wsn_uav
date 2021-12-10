#include <iostream>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <list>
#include <algorithm>
#include "evolution.h"
#include "solution.h"
#include "problem.h"
#include <chrono> 
#include <cfloat>
#include <fstream>
using namespace std::chrono; 

using namespace std;

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char** argv) {
	clock_t start = clock();
	unsigned int seed = (unsigned int)time(NULL);
	
	// printf("Seed = %d\n", seed);
	srand(seed);

	// load input data
	// cout << "load input data";
	// for (int jj=0; jj<-1; jj++) module.trace() << "load input data";
	problem input;
	int numUAVs = 1;
	input.load2("att48_xy.txt");
	
	// initial population
	// cout << "initial population";
	cout << "initial population" << endl;
	list<solution> population(100);
	for(list<solution>::iterator it = population.begin(); it != population.end(); ++it){
		it->random(numUAVs, input);
	}

	ofstream minDistanceFile;
  	// minDistanceFile.open ("MinDistance.txt");
  	// minDistanceFile.close();
  	ofstream avgDistanceFile;
  	// avgDistanceFile.open ("AvgDistance.txt");
  	// avgDistanceFile.close;

  	minDistanceFile.open("MinDistance.txt", std::ios_base::app);
	avgDistanceFile.open("AvgDistance.txt", std::ios_base::app);
	minDistanceFile << "\n";
	avgDistanceFile << "\n";
	minDistanceFile.close();
	avgDistanceFile.close();
	
	// evolution
	// cout << "evolution";
	cout << "evolution" << endl;
	for(int run = 0; run < 200; run++){
		auto start = high_resolution_clock::now(); 
		// cout << "evolution = " << run;
		cout << "evolution = " << run << ", population " << population.size() << endl;
		// for (solution sol : population) {
		// 	module.trace() << "population fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
		// }
		// putchar('*');

		cout << "crossover = " << run << ", population " << population.size() << endl;
		list<solution> merged(population.begin(), population.end() );
		for(int off = 0; off < 50; off++){
			solution parent1 = tournament(population, input);
			solution parent2 = tournament(population, input);

			solution off1 = crossover(parent1, parent2, input);
			merged.push_back(off1);
			solution off2 = crossover(parent2, parent1, input);
			merged.push_back(off2);
		}
		cout << "merge size = " << merged.size() << endl;

		// for (int ii=0; ii<-10; ii++) module.trace() << "*****************";
		// for (solution sol : merged) {
		// 	module.trace() << "crossover fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
		// }

		cout << "mutation = " << run << ", population " << population.size() << endl;
		cout << "merge size = " << merged.size() << endl;
		list<solution> feasible, infeasible;
		for(list<solution>::iterator it = merged.begin(); it != merged.end(); it++){
			mutation(*it, input);

			if(it->feasible) feasible.push_front(*it);
			else infeasible.push_front(*it);
		}


		// for (int ii=0; ii<-10; ii++) module.trace() << "*****************";
		// for (solution sol : merged) {
		// 	module.trace() << "mutation fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
		// }

		cout << "ranking = " << run << ", population " << population.size() << endl;
		vector< list<solution> > frank, irank;
		ranking(feasible, &frank, true);
		ranking(infeasible, &irank, false);

		cout << "frank " << frank.size() << " irank " << irank.size() << endl;

		// for (int ii=0; ii<-10; ii++) module.trace() << "*****************";
		int count  = 0;
		double minDistance = DBL_MAX;
		double avgDistance = 0;
		for (auto listsol : frank) {
			for (auto sol : listsol) {
				// cout << "feasible fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance << endl;
				// if (count-- == 0) break;
				count++;
				if (sol.maxDistance < minDistance) minDistance = sol.maxDistance;
				avgDistance += sol.maxDistance;
			}
		}
		count = 10;
		for (auto listsol : irank) {
			for (auto sol : listsol) {
				// cout << "infeasible fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance << endl;
				// if (count-- == 0) break;
				count++;
				if (sol.maxDistance < minDistance) minDistance = sol.maxDistance;
				avgDistance += sol.maxDistance;
			}
		}
		avgDistance /= count;
		minDistanceFile.open ("MinDistance.txt", std::ios_base::app);
		avgDistanceFile.open ("AvgDistance.txt", std::ios_base::app);
		minDistanceFile << minDistance << "\t";
		avgDistanceFile << avgDistance << "\t";
		minDistanceFile.close();
		avgDistanceFile.close();


		cout << "environmental = " << run << ", population " << population.size() << endl;
		population.clear();
		environmental(frank, irank, &population, 200);

		// if ((!frank.empty()) && ((frank[0]).front().maxDistance < maxDelay)) break;

		for (int ii=0; ii<10; ii++) {
			cout << irank[ii].front().maxDistance << " " << std::next(population.begin(), ii)->maxDistance << endl;
		}

		auto stop = high_resolution_clock::now(); 
		auto duration = duration_cast<microseconds>(stop - start);
		
		cout << "generation " << run << " " << duration.count() << endl;
	}
	// putchar('\n');


	// finally...
	// for (int jj=0; jj<-1; jj++) module.trace() << "finally...";
	vector< list<solution> > rank;
	ranking(population, &rank, true);
	// for (int jj=0; jj<-1; jj++) module.trace() << "ranking...";
	rank[0].sort();
	// for (int jj=0; jj<-1; jj++) module.trace() << "sort...";
	// // rank[0].unique();
	// for (int jj=0; jj<-1; jj++) module.trace() << "unique...";

	// for(list<solution>::const_iterator it = rank[0].begin(); it != rank[0].end(); ++it)
	// 	// if(it->feasible) it->print(stdout);
	// 	it->print(input);
	// clock_t end = clock();

	// printf("%.3f second\n", ((double)(end)-start) / CLOCKS_PER_SEC);
	// for (int jj=0; jj<-1; jj++) module.trace() << "maxDistance " << (rank[0]).front().maxDistance;

	int numVisits = (rank[0]).front().validateNumVisits(input);
	cout << "Final solution " << numVisits << " " << input.getNumCusto() << " " << (rank[0]).front().routes.size() << endl;

//	return (rank[0]).front().toTrajectories(input);
	return 0;
}
