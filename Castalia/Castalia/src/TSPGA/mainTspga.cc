#include "mainTspga.h"

vector<vector<int>> callTspga(CastaliaModule &module, int sinkId, std::vector<int> landmarks, int numUAVs, double maxDelay){

	clock_t start = clock();
	unsigned int seed = (unsigned int)time(NULL);
	
	// printf("Seed = %d\n", seed);
	srand(seed);

	// load input data
	// cout << "load input data";
	// for (int jj=0; jj<0; jj++) module.trace() << "load input data";
	problem_tsp::module = (&module);
	problem_tsp input;
	input.load(sinkId, landmarks, numUAVs, maxDelay);
	
	// initial population
	// cout << "initial population";
	for (int jj=0; jj<0; jj++) module.trace() << "initial population";
	solution_tsp::module = (&module);
	route_tsp::module = (&module);
	list<solution_tsp> population(200);
	for(list<solution_tsp>::iterator it = population.begin(); it != population.end(); ++it){
		it->random(input);
	}
	
	// evolution_tsp
	// cout << "evolution_tsp";
	for (int jj=0; jj<0; jj++) module.trace() << "evolution_tsp";
	for(int run = 0; run < 150; run++){
		// cout << "evolution_tsp = " << run;
		for (int jj=0; jj<0; jj++) module.trace() << "evolution_tsp = " << run;
		// for (solution_tsp sol : population) {
		// 	module.trace() << "population fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
		// }
		// putchar('*');

		for (int jj=0; jj<0; jj++) module.trace() << "crossover " << run;
		list<solution_tsp> merged(population.begin(), population.end() );
		for(int off = 0; off < 50; off++){
			solution_tsp parent1 = tournament(population, input);
			solution_tsp parent2 = tournament(population, input);

			solution_tsp off1 = crossover(parent1, parent2, input);
			merged.push_back(off1);
			solution_tsp off2 = crossover(parent2, parent1, input);
			merged.push_back(off2);
		}

		// for (int ii=0; ii<10; ii++) module.trace() << "*****************";
		// for (solution_tsp sol : merged) {
		// 	module.trace() << "crossover fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
		// }

		for (int jj=0; jj<0; jj++) module.trace() << "mutation " << run;
		list<solution_tsp> feasible, infeasible;
		for(list<solution_tsp>::iterator it = merged.begin(); it != merged.end(); it++){
			mutation(*it, input);

			if(it->feasible) feasible.push_front(*it);
			else infeasible.push_front(*it);
		}

		// for (int ii=0; ii<10; ii++) module.trace() << "*****************";
		// for (solution_tsp sol : merged) {
		// 	module.trace() << "mutation fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
		// }

		for (int jj=0; jj<0; jj++) module.trace() << "ranking " << run;
		for (int jj=0; jj<0; jj++) module.trace() << "feasible " << feasible.size() << " infeasible " << infeasible.size();
		vector< list<solution_tsp> > frank, irank;
		ranking(feasible, &frank, true);
		ranking(infeasible, &irank, false);

		if (!frank.empty() && !frank[0].empty()) {
			frank[0].sort();
			solution_tsp::module->trace() << "best solution " << landmarks.size() << " " << (frank[0]).front().maxDistance;
			// stringstream ss;
			// for(int q : (frank[0]).front().route.visits) {
			// 	ss << q << " ";
			// }
			// solution_tsp::module->trace() << ss.str();
		}

		// for (int ii=0; ii<10; ii++) module.trace() << "*****************";
		// for (solution_tsp sol : feasible) {
		// 	module.trace() << "feasible fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
		// }
		// for (solution_tsp sol : infeasible) {
		// 	module.trace() << "infeasible fitness " << sol.routes.size() << " " << sol.maxDistance << " " << sol.totalDistance;
		// }

		population.clear();
		for (int jj=0; jj<0; jj++) module.trace() << "environmental " << run;
		environmental(frank, irank, &population, 200);

		if ((!frank.empty()) && ((frank[0]).front().maxDistance < maxDelay)) break;
	}
	// putchar('\n');

	// finally...
	// for (int jj=0; jj<0; jj++) module.trace() << "finally...";
	vector< list<solution_tsp> > rank;
	ranking(population, &rank, true);
	// for (int jj=0; jj<0; jj++) module.trace() << "ranking...";
	rank[0].sort();
	// for (int jj=0; jj<0; jj++) module.trace() << "sort...";
	// // rank[0].unique();
	// for (int jj=0; jj<0; jj++) module.trace() << "unique...";

	// for(list<solution_tsp>::const_iterator it = rank[0].begin(); it != rank[0].end(); ++it)
	// 	// if(it->feasible) it->print(stdout);
	// 	it->print(input);
	// clock_t end = clock();

	// printf("%.3f second\n", ((double)(end)-start) / CLOCKS_PER_SEC);
	// for (int jj=0; jj<0; jj++) module.trace() << "maxDistance " << (rank[0]).front().maxDistance;

	int numVisits = (rank[0]).front().route.visits.size();
	for (int jj=0; jj<0; jj++) module.trace() << "Final solution_tsp " << numVisits << " " << input.getNumCusto();

	return (rank[0]).front().toTrajectories(input);
}