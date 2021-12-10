#include <vector>
#include <algorithm>
#include "evolution_tsp.h"

using namespace std;

solution_tsp crossover(const solution_tsp &pa, const solution_tsp &pb, const problem_tsp& input){
	solution_tsp offspring;
	offspring.route.visits = vector<int>();
	// offspring.clear();

	vector<bool> isTracked (input.getNumCusto()+1, false);

	vector<int> index_a (input.getNumCusto()+1);
	vector<int> index_b (input.getNumCusto()+1);
	for (int i=0; i<input.getNumCusto(); i++) {
		index_a[pa.route.visits[i]] = i;
		index_b[pb.route.visits[i]] = i;
	}

	int current_node = pa.route.visits[0];
	isTracked[current_node] = true;
	offspring.route.visits.push_back(current_node);
	while (offspring.route.visits.size() < input.getNumCusto()) {
		int node_alpha, node_beta;
		for (int i_a=1; i_a<input.getNumCusto(); i_a++) {
			int id = (i_a + index_a[current_node]) % input.getNumCusto();
			if (isTracked[pa.route.visits[id]]) continue;
			else {
				node_alpha = pa.route.visits[id];
				break;
			}
		}
		for (int i_b=1; i_b<input.getNumCusto(); i_b++) {
			int id = (i_b + index_b[current_node]) % input.getNumCusto();
			if (isTracked[pb.route.visits[id]]) continue;
			else {
				node_beta = pb.route.visits[id];
				break;
			}
		}
		double d_a = input.getDistance(current_node, node_alpha);
		double d_b = input.getDistance(current_node, node_beta);
		if (d_a < d_b) {
			current_node = node_alpha;
		} else {
			current_node = node_beta;
		}
		offspring.route.visits.push_back(current_node);
		isTracked[current_node] = true;
	}


	offspring.route.modified = true;
	offspring.fitness(input);

	if (offspring == pa) {
		solution_tsp::module->trace() << "bad crossover";
		stringstream ss;
		for(int q : pa.route.visits) {
			ss << q << " ";
		}
		solution_tsp::module->trace() << ss.str();
		stringstream ss2;
		for(int q : pb.route.visits) {
			ss << q << " ";
		}
		solution_tsp::module->trace() << ss2.str();
	}
	
	return offspring;
}

void mutation(solution_tsp &sol, const problem_tsp& input){
	// int p = rand() % 100;
	// if (p > 99) return;
	// int selectA = rand() % input.getNumCusto();
	// int selectB = rand() % input.getNumCusto();

	// solution_tsp temp = sol;
	// int tmp = temp.route.visits[selectA];
	// temp.route.visits[selectA] = temp.route.visits[selectB];
	// temp.route.visits[selectB] = tmp;

	// temp.route.modified = true;
	// temp.fitness(input);

	// if (temp.maxDistance < sol.maxDistance) sol = temp;
	int tryCount = 0;
	while(tryCount < input.getNumCusto() ){
		solution_tsp test = sol;
		int selectA = rand() % input.getNumCusto();
		int selectB = rand() % input.getNumCusto();
		int tmp = test.route.visits[selectA];
		test.route.visits[selectA] = test.route.visits[selectB];
		test.route.visits[selectB] = tmp;
		test.route.modified = true;
		test.fitness(input);

		if(test.maxDistance < sol.maxDistance) sol = test;

		tryCount++;
	}
}

// 2-tournament selection from population
const solution_tsp& tournament(const std::list<solution_tsp> &population, const problem_tsp &input){
	int selectA = rand() % population.size();
	int selectB = rand() % population.size();

	list<solution_tsp>::const_iterator itA = population.begin();
	advance(itA, selectA);
	list<solution_tsp>::const_iterator itB = population.begin();
	advance(itB, selectB);

	int cmp = solution_tsp::cmp( (*itA), (*itB), input);
	if(cmp == 0) cmp = (rand() % 2) * 2 - 1;   // -1 or 1

	if(cmp < 0) return (*itA);
	else return (*itB);
}

// Use Deb's "Fast Nondominated Sorting" (2002)
// Ref.: "A fast and elitist multiobjective genetic algorithm: NSGA-II"
void ranking(const std::list<solution_tsp> &population, std::vector< std::list<solution_tsp> > *output, bool feasible){
	if( population.empty() ) return;
	vector<solution_tsp> solution_tsps(population.begin(), population.end() );

	vector< list<int> > intOutput;
	intOutput.resize(solution_tsps.size() + 2);  // start from 1, end with null Qi 
	output->resize(1);

	// record each solution_tsps' dominated solution_tsp
	vector< list<int> > dominated;
	dominated.resize(solution_tsps.size());

	// record # of solution_tsps dominate this solution_tsp
	vector<int> counter;
	counter.resize(solution_tsps.size());

	// record the rank of solution_tsps
	vector<int> rank;
	rank.resize(solution_tsps.size());

	
	// for each solution_tsp
	for(unsigned int p = 0; p < solution_tsps.size(); p++){
		for(unsigned int q = 0; q < solution_tsps.size(); q++){
			if( feasible ){
				if( solution_tsp::fdominate(solution_tsps[p], solution_tsps[q]) ){
					dominated[p].push_back(q);  // Add q to the set of solution_tsps dominated by p
				}else if( solution_tsp::fdominate(solution_tsps[q], solution_tsps[p]) ){
					counter[p]++;
				}
			}else{
				if( solution_tsp::idominate(solution_tsps[p], solution_tsps[q]) ){
					dominated[p].push_back(q);  // Add q to the set of solution_tsps dominated by p
				}else if( solution_tsp::idominate(solution_tsps[q], solution_tsps[p]) ){
					counter[p]++;
				}
			}
		}

		// solution_tsp::module->trace() << "solution " << p << " distance " << solution_tsps[p].maxDistance << " counter " << counter[p];
		// stringstream ss;
		// for(int q : dominated[p]) {
		// 	ss << q << " " << solution_tsps[q].maxDistance << " ";
		// }
		// solution_tsp::module->trace() << ss.str();
		
		if(counter[p] == 0){  // p belongs to the first front
			rank[p] = 1;
			intOutput[1].push_back(p);
			(*output)[0].push_back(solution_tsps[p]);
		}
	}

	int curRank = 1;
	while( intOutput[curRank].size() > 0 ){
		list<int> Qi;  // Used to store the members of the next front
		list<solution_tsp> Qs;

		for(list<int>::iterator p = intOutput[curRank].begin(); p != intOutput[curRank].end(); p++){
		for(list<int>::iterator q = dominated[(*p)].begin(); q != dominated[(*p)].end(); q++){
			counter[(*q)]--;
			if(counter[(*q)] == 0){  // q belongs to the next front
				rank[(*q)] = curRank + 1;
				Qi.push_back(*q);
				Qs.push_back(solution_tsps[(*q)]);
			}
		}}

		curRank++;
		intOutput[curRank] = Qi;
		if(Qi.size() > 0) {
			output->push_back(Qs);
			// solution_tsp::module->trace() << "rank " << curRank;
			// solution_tsp::module->trace() << "Qs " << Qs.size();
			// stringstream ss;
			// for(int q : Qi) {
			// 	ss << q << " ";
			// }
			// solution_tsp::module->trace() << ss.str();
		}
	}

	// remove duplicate solution_tsp in same rank
	for(unsigned int rank = 0; rank < output->size(); ++rank){
		(*output)[rank].sort();
		(*output)[rank].unique();
	}
}

void environmental(const vector< list<solution_tsp> > &frank, const vector< list<solution_tsp> > &irank, list<solution_tsp> *output, unsigned int maxSize){
	unsigned int curRank = 0;
	// solution_tsp::module->trace() << "frank " << frank.size() << " irank " << irank.size();

	while(true){
		if(curRank < frank.size() && output->size() + frank[curRank].size() <= maxSize){
			output->insert(output->end(), frank[curRank].begin(), frank[curRank].end() );
		}else if(curRank < frank.size() ) break;

		if(curRank < irank.size() && output->size() + irank[curRank].size() <= maxSize){
			output->insert(output->end(), irank[curRank].begin(), irank[curRank].end() );
		}else if(curRank < irank.size() ) break;

		if ((curRank > frank.size() ) && (curRank > irank.size() )) break;

		curRank++;
	}

	// solution_tsp::module->trace() << "output size " << output->size();
	
	if(output->size() < maxSize && curRank < frank.size() ){
		vector<solution_tsp> nextRank(frank[curRank].begin(), frank[curRank].end() );

		while(output->size() < maxSize && !nextRank.empty()){
			unsigned int select = rand() % nextRank.size();
			output->push_back(nextRank[select]);
			nextRank.erase(nextRank.begin() + select);
		}
	}
	if(output->size() < maxSize && curRank < irank.size() ){
		vector<solution_tsp> nextRank(irank[curRank].begin(), irank[curRank].end() );

		while(output->size() < maxSize && !nextRank.empty()){
			unsigned int select = rand() % nextRank.size();
			output->push_back(nextRank[select]);
			nextRank.erase(nextRank.begin() + select);
		}
	}

}