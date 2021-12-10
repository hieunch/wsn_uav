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
#include <fstream>
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
		// problem::module = (&module);
		problem input;
		input.load(sinkId, landmarks, numUAVs, maxDelay);
		
		// initial population
		// cout << "initial population";
		for (int jj=0; jj<-1; jj++) module.trace() << "initial population";
		// solution::module = (&module);
		// route::module = (&module);
		list<solution> population(200);
		for(list<solution>::iterator it = population.begin(); it != population.end(); ++it){
			it->random(numUAVs, input);
			it->generation = 0;
		}

		ofstream minDistanceFile;
  		ofstream avgDistanceFile;
		minDistanceFile.open("MinDistance.txt", std::ios_base::app);
		avgDistanceFile.open("AvgDistance.txt", std::ios_base::app);
		minDistanceFile << "\n";
		avgDistanceFile << "\n";
		minDistanceFile.close();
		avgDistanceFile.close();
		
		// evolution
		// cout << "evolution";
		// for (int jj=0; jj<-1; jj++) module.trace() << "evolution";
		for(int run = 0; run < 400; run++){
			auto start = high_resolution_clock::now(); 
			// cout << "evolution = " << run;
			// for (int ii=0; ii<-1; ii++) module.trace() << "evolution = " << run << ", population " << population.size();
			// for (solution sol : population) {
			// 	module.trace() << "population fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
			// }
			// putchar('*');

			// for (int ii=0; ii<-1; ii++) module.trace() << "crossover = " << run << ", population " << population.size();
			list<solution> merged(population.begin(), population.end() );
			for(int off = 0; off < 100; off++){
				solution parent1 = tournament(population, input);
				solution parent2 = tournament(population, input);

				solution off1 = crossover(parent1, parent2, input);
				// if (parent1.maxDistance == off1.maxDistance && parent1.totalDistance == off1.totalDistance) 
				// 	off1.generation = parent1.generation;
				// else if (parent2.maxDistance == off1.maxDistance && parent2.totalDistance == off1.totalDistance) 
				// 	off1.generation = parent2.generation;
				// else 
					off1.generation = run;
				merged.push_back(off1);
				solution off2 = crossover(parent2, parent1, input);
				// if (parent1.maxDistance == off2.maxDistance && parent1.totalDistance == off2.totalDistance) 
				// 	off2.generation = parent1.generation;
				// else if (parent2.maxDistance == off2.maxDistance && parent2.totalDistance == off2.totalDistance) 
				// 	off2.generation = parent2.generation;
				// else 
					off2.generation = run;
				merged.push_back(off2);
			}
			// for (int ii=0; ii<-1; ii++) module.trace() << "merge size = " << merged.size();

			// for (int ii=0; ii<-10; ii++) module.trace() << "*****************";
			// for (solution sol : merged) {
			// 	module.trace() << "crossover fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
			// }

			// for (int ii=0; ii<-1; ii++) module.trace() << "mutation = " << run << ", population " << population.size();
			// for (int ii=0; ii<-1; ii++) module.trace() << "merge size = " << merged.size();
			list<solution> feasible, infeasible;
			for(list<solution>::iterator it = merged.begin(); it != merged.end(); it++){
				mutation(*it, input, run);

				if(it->feasible) feasible.push_front(*it);
				else infeasible.push_front(*it);
			}

			// for (int ii=0; ii<-10; ii++) module.trace() << "*****************";
			// for (solution sol : merged) {
			// 	module.trace() << "mutation fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
			// }

			// for (int ii=0; ii<-10; ii++) module.trace() << "ranking = " << run << ", population " << population.size();
			vector< list<solution> > frank, irank;
			ranking(feasible, &frank, true);
			ranking(infeasible, &irank, false);

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

			// for (int ii=0; ii<-1; ii++) module.trace() << "frank " << frank.size() << " irank " << irank.size();

			// for (int ii=0; ii<-10; ii++) module.trace() << "*****************";
			// for (solution sol : feasible) {
			// 	module.trace() << "feasible fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
			// }
			// for (solution sol : infeasible) {
			// 	module.trace() << "infeasible fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
			// }

			for (int ii=0; ii<-10; ii++) module.trace() << "environmental = " << run << ", population " << population.size();

			int oldestGen = run-10;
			for (auto listsol : irank) {
				for (auto sol : listsol) {
					if (sol.generation < oldestGen) oldestGen = sol.generation;
				}
			}
			vector< list<solution> > new_irank;
			for (auto listsol : irank) {
				list<solution> new_list;
				for (solution sol : listsol) {
					if (sol.generation != oldestGen) new_list.push_back(sol);
				}
				new_irank.push_back(new_list);
			}
			irank = new_irank;

			population.clear();
			environmental(frank, irank, &population, 200);

			// if ((!frank.empty()) && ((frank[0]).front().maxDistance < maxDelay)) break;

			auto stop = high_resolution_clock::now(); 
			auto duration = duration_cast<microseconds>(stop - start);
			
			// module.trace() << "generation " << run << " " << duration.count();
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
		// module.trace() << "Final solution " << numVisits << " " << input.getNumCusto() << " " << (rank[0]).front().routes.size();

		return (rank[0]).front().toTrajectories(input);
	}

};

