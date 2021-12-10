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
#include "CastaliaModule.h"
#include <chrono> 
using namespace std::chrono; 

using namespace std;

class Vrptw {
  public:
    static vector<vector<int>> call(CastaliaModule &module, int sinkId, std::vector<int> landmarks, int numUAVs, double maxDelay){
		clock_t start = clock();
		unsigned int seed = (unsigned int)time(NULL);
		
		// printf("Seed = %d\n", seed);
		srand(seed);

		// load input data
		// cout << "load input data";
		// for (int jj=0; jj<-1; jj++) module.trace() << "load input data";
		problem::module = (&module);
		problem input;
		input.load(sinkId, landmarks, numUAVs, maxDelay);
		
		// initial population
		// cout << "initial population";
		for (int jj=0; jj<-1; jj++) module.trace() << "initial population";
		solution::module = (&module);
		route::module = (&module);
		list<solution> population(200);
		for(list<solution>::iterator it = population.begin(); it != population.end(); ++it){
			it->random(numUAVs, input);
		}
		
		// evolution
		// cout << "evolution";
		for (int jj=0; jj<-1; jj++) module.trace() << "evolution";
		for(int run = 0; run < 400; run++){
			auto start = high_resolution_clock::now(); 
			// cout << "evolution = " << run;
			for (int ii=0; ii<-1; ii++) module.trace() << "evolution = " << run << ", population " << population.size();
			// for (solution sol : population) {
			// 	module.trace() << "population fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
			// }
			// putchar('*');

			for (int ii=0; ii<-1; ii++) module.trace() << "crossover = " << run << ", population " << population.size();
			list<solution> merged(population.begin(), population.end() );
			for(int off = 0; off < 50; off++){
				solution parent1 = tournament(population, input);
				solution parent2 = tournament(population, input);

				solution off1 = crossover(parent1, parent2, input);
				merged.push_back(off1);
				solution off2 = crossover(parent2, parent1, input);
				merged.push_back(off2);
			}
			for (int ii=0; ii<-1; ii++) module.trace() << "merge size = " << merged.size();

			// for (int ii=0; ii<-10; ii++) module.trace() << "*****************";
			// for (solution sol : merged) {
			// 	module.trace() << "crossover fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
			// }

			for (int ii=0; ii<-1; ii++) module.trace() << "mutation = " << run << ", population " << population.size();
			for (int ii=0; ii<-1; ii++) module.trace() << "merge size = " << merged.size();
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

			for (int ii=0; ii<-10; ii++) module.trace() << "ranking = " << run << ", population " << population.size();
			vector< list<solution> > frank, irank;
			ranking(feasible, &frank, true);
			ranking(infeasible, &irank, false);

			for (int ii=0; ii<-1; ii++) module.trace() << "frank " << frank.size() << " irank " << irank.size();

			// for (int ii=0; ii<-10; ii++) module.trace() << "*****************";
			// for (solution sol : feasible) {
			// 	module.trace() << "feasible fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
			// }
			// for (solution sol : infeasible) {
			// 	module.trace() << "infeasible fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
			// }

			for (int ii=0; ii<-10; ii++) module.trace() << "environmental = " << run << ", population " << population.size();
			population.clear();
			environmental(frank, irank, &population, 200);

			// if ((!frank.empty()) && ((frank[0]).front().maxDistance < maxDelay)) break;

			auto stop = high_resolution_clock::now(); 
			auto duration = duration_cast<microseconds>(stop - start);
			
			module.trace() << "generation " << run << " " << duration.count();
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
		module.trace() << "Final solution " << numVisits << " " << input.getNumCusto() << " " << (rank[0]).front().routes.size();

		return (rank[0]).front().toTrajectories(input);
	}

};

