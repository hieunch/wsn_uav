//****************************************************************************
//*  Copyright (c) Federal University of Para, brazil - 2011                 *
//*  Developed at the Research Group on Computer Network and Multimedia      *
//*  Communication (GERCOM)     				             *
//*  All rights reserved    				                     *
//*                                                                          *
//*  Permission to use, copy, modify, and distribute this protocol and its   *
//*  documentation for any purpose, without fee, and without written         *
//*  agreement is hereby granted, provided that the above copyright notice,  *
//*  and the author appear in all copies of this protocol.                   *
//*                                                                          *
//*  Module:   LEACH Clustering Protocol for Castalia Simulator              *
//*  Version:  0.2                                                           *
//*  Author(s): Adonias Pires <adonias@ufpa.br>                              *
//*             Claudio Silva <claudio.silva@itec.ufpa.br>                   *
//****************************************************************************/

#include "GAMBACBalanceNew.h"

Define_Module(GAMBACBalanceNew);


// void GAMBACBalanceNew::timerFiredCallback(int index)
// {
// 	switch (index) {
// 		case START_MAINALG:{	
// 			uncollectedSet.clear();
// 			collectedSet.clear();
// 			for (int i=0; i<N; i++) if (isAliveSN(i)) uncollectedSet.insert(i);
// 			totalConsumed = 0;
// 			maxConsumed = 0;
// 			maxTxSize = 0;
// 			energyConsumeds = vector<double>(N, 0);
// 			rxSizes = vector<double>(N, 0);
// 			auto start = std::chrono::steady_clock::now();
// 			totalCollected = 0;
// 			mainAlg();
// 			logConfig();
// 			auto end = std::chrono::steady_clock::now();
// 			time_elapse = end - start;
// 			break;
// 		}
// 		default:{
// 			GAMBAC::timerFiredCallback(index);
// 		}
// 	}
// }

// void GAMBACBalanceNew::mainAlg() {

// 	for (int u=0; u<numNodes; u++) E_current[u] = getResMgrModule(u)->getRemainingEnergy();
// 	config.A.clear();

// 	for (int i=0; i<10; i++) trace() << "mainAlg";
// 	W_total = 0;
// 	for (int i=0; i<numNodes; i++) W_total += dataVolume[i];
// 	W_start = W_total/numUAVs/2;
// 	W_end = 1000;

// 	W = (W_start+W_end)/2;

// 	bool lengthViolated;
// 	bool isSaved = false;
// 	int retry = 0;
// 	double ratioToLmax = 1;
// 	double W_saved;
// 	double maxLen = 0;
	
// 	do {
// 		reset();

// 		routing_trace() << "W " << W << " W_start " << W_start << " W_end " << W_end;
// 		routing_trace() << "selectCHsAndBuildDFT";
// 		int loop_count = selectCHsAndBuildDFT(W);
// 		routing_trace() << "buildTrajectories " << A.size();
// 		if (A == config.A) {
// 			trajectories = config.trajectories;
// 		}
// 		else buildTrajectories(true);
// 		lengthViolated = false;
// 		maxLen = 0;
			
// 		// verify trajectories length
// 		for (int t=0; t<numUAVs; t++) {
// 			double len = calculatePathLength(trajectories[t]);
// 			if (len > L_max) {
// 				lengthViolated = true;
// 			};
// 			if (len > maxLen) maxLen = len;
// 		}
// 		ratioToLmax = pow(maxLen / L_max, 2);
// 		routing_trace() << "maxLen " << maxLen << " ratioToLmax " << ratioToLmax;


// 		if (lengthViolated) {
// 			// Increase W
// 			retry += 1;
// 			if (retry >= 5) {
// 				W_end = W;
// 				W = W*ratioToLmax;
// 				retry = 0;
// 			}
// 			else continue;
// 		} else {
// 			if (A.size() > config.A.size() && loop_count <= 50){
// 				routing_trace() << "save config";
// 				config.save(A, clus_id, nextHop, trajectories);
// 				isSaved = true;
// 				W_start = W;
// 				W = W*ratioToLmax;
// 			}
// 			else {
// 				retry += 1;
// 				if (retry >= 5) {
// 					if (!isSaved) {
// 						config.save(A, clus_id, nextHop, trajectories);
// 						isSaved = true;
// 					}
// 					W_start = W;
// 					W = W*ratioToLmax;
// 					retry = 0;
// 				}
// 				continue;
// 			}
			
// 			retry = 0;
// 		}

// 		if (W_start - W <= 10000) break;
		
		
// 		if ((W_start-W_end <= 10000) && !isSaved) {
// 			W_start = W_total/numUAVs/2;
// 			W_end = 1000;
// 			W_opt = -1;
// 		}

// 	} while (W_start-W_end>10000);

	
// 	A = config.A;
// 	stringstream ssA;
// 	for(int l : A) {
// 		ssA << l << " ";
// 	}
// 	routing_trace() << "CH list: " << ssA.str();

// 	///////////////////////////////////////////////////////////

// 	double maxLength = 0;
// 	for (auto T : config.trajectories) {
// 		double length = calculatePathLength(T);
// 		if (length > maxLength) maxLength = length;

// 		stringstream ss;
// 		for(int l : T) {
// 			ss << l << " ";
// 		}
// 		routing_trace() << ss.str();
// 	}
// 	routing_trace() << "maxLength " << maxLength;
// 	maxLengthRatio = maxLength / L_max;
// 	routing_trace() << "END mainAlg";
// }


// int GAMBACBalanceNew::selectCHsAndBuildDFT(double W){
// 	E_min = DBL_MAX;
// 	E_max = 0;
// 	for (int i=0; i<numNodes; i++) {
// 		if (!isAliveSN(i)) continue;
// 		if (E_current[i] > E_max) E_max = E_current[i];
// 		if (E_current[i] < E_min) E_min = E_current[i];
// 	}

// 	for (int i=0; i<N; i++) {
// 		if (!isAliveSN(i)) continue;
// 		w_max[i] = W;
// 	}
	
// 	int k_max = (int) ((1 + epsilon) * W_total / W);
// 	routing_trace() << "k_max " << k_max;
	
// 	int count = 0;
// 	vector<int> allSNs;
// 	for (int i=0; i<N; i++) {
// 		if (!isAliveSN(i)) continue;
// 		allSNs.push_back(i);
// 	}
// 	while (A.empty()) {
// 		A = randomFromSet(allSNs, 8);
// 	}
// 	routing_trace() << "initial size " << A.size();
// 	// growBalls(A);
// 	bool isBalance;
// 	double mineps = DBL_MAX;
// 	vector<int> A_min;
// 	do {
// 		growBalls(A);
// 		int Asize = A.size();
// 		recruitNewCHs();
// 		// routing_trace() << "growBalls " << A.size();
// 		growBalls(A);

// 		double maxWeight = 0;
// 		double totalWeight = 0;
// 		for (auto pair : ballWeight) {
// 			if (maxWeight < pair.second) {
// 				maxWeight = pair.second;
// 			}
// 			totalWeight += pair.second;
// 		}

// 		// routing_trace() << "getOuterOversizePart() " << getOuterOversizePart().size();
// 		// routing_trace() << "count " << count << " maxWeight " << maxWeight << " W " << W << " totalWeight " << totalWeight << " W_total " << W_total;

// 		double eps = maxWeight/W_total*A.size() - 1;
// 		routing_trace() << "count " << count << " Asize " << A.size() << " epsilon " << eps;
// 		if (eps < mineps) {
// 			mineps = eps;
// 			A_min = A;
// 		}

// 		vector<int> A_new;
// 		for (auto pair : ballWeight) {
// 			if (totalWeight/A.size() < pair.second) {
// 				A_new.push_back(pair.first);
// 			}
// 		}
// 		A = A_new;

// 		if (count++ > 50) {
// 			break;
// 		}

// 		countAdjusment++;
// 		nloop = count;
// 	} while (!isBalance);

// 	A = A_min;
// 	growBalls(A);
// 	double maxWeight = 0;
// 	for (auto pair : ballWeight) {
// 		if (pair.second > maxWeight) maxWeight = pair.second;
// 	}
// 	routing_trace() << "epsilon " << maxWeight/W_total*A.size() - 1 << " count " << count;
// 	clus_id[self] = -1;
// 	for (int l : A) clus_id[l] = -1;
// 	for (int i=0; i<1; i++) trace() << "A size " << A.size();
// 	trace() << "W " << W;
// 	trace() << "k_max " << k_max;
// 	return count;
// }


// // int GAMBACBalanceNew::selectCHsAndBuildDFT(double W){
	
// // 	A.clear();
// // 	unordered_set<int> uncovered_nodes;
// // 	int new_CH;
// // 	double maxE = 0;
// // 	for (int u=0; u<numNodes; u++) {
// // 		if (isAliveSN(u)) uncovered_nodes.insert(u);
// // 		if (E_current[u] > maxE) {
// // 			maxE = E_current[u];
// // 			new_CH = u;
// // 		}
// // 	}
// // 	/* auto it = std::begin(uncovered_nodes);
// // 	auto r = rand() % uncovered_nodes.size();
// // 	std::advance(it,r); */

// // 	while (!uncovered_nodes.empty()) {
		
// // 		// routing_trace() << "Asize " << A.size() << " uncovered_nodes.size " << uncovered_nodes.size();

// // 		vector<int> Candidates;
// // 		vector<int> countCloseCHs(N, 0);
// // 		vector<bool> isCovered(N, false);
// // 		for (int CH : A) {
// // 			vector<int> S_CH;
// // 			S_CH.push_back(CH);
// // 			growBallsWeightBounded(S_CH, 4*W);
// // 			unordered_set<int> covered_nodes;
// // 			for (int u=0; u<numNodes; u++) {
// // 				if (isAliveSN(u) && clus_id[u] != -1) covered_nodes.insert(u);
// // 			}
// // 			for (int u=0; u<numNodes; u++) {
// // 				if (nextHop[u] != -1) covered_nodes.erase(nextHop[u]);
// // 			}
// // 			for (int u : covered_nodes) countCloseCHs[u]++;
// // 			/* for (int u : uncovered_nodes) {
// // 				// if (clus_id[u] != -1) isCovered[u] = true;
// // 				if (isCovered[u]) continue;
// // 				for (int v : graph.getAdjExceptSink(u)) {
// // 					if (clus_id[v] != -1) {
// // 						countCloseCHs[u]++;
// // 						break;
// // 					}
// // 				}
// // 			} */
// // 		}
// // 		for (int u : uncovered_nodes) {
// // 			if (!isCovered[u] && countCloseCHs[u] >= 2) {
// // 				Candidates.push_back(u);
// // 				// routing_trace() << "node " << u << " countCloseCHs " << countCloseCHs[u];
// // 			}
// // 			// if (!isCovered[u]) routing_trace() << "node " << u << " countCloseCHs " << countCloseCHs[u];
// // 		}
// // 		// routing_trace() << "Candidates size " << Candidates.size();
// // 		// int newCH = -1;
// // 		/* if (!Candidates.empty()) {
// // 			auto it2 = std::begin(Candidates);
// // 			auto r2 = rand() % Candidates.size();
// // 			std::advance(it2,r2);
// // 			new_CH = *it2;
// // 		}  */
// // 		int maxClose = 0;
// // 		double maxE = 0;
// // 		for (int u : uncovered_nodes) {
// // 			if (countCloseCHs[u] > maxClose) {
// // 				maxClose = countCloseCHs[u];
// // 				maxE = E_current[u];
// // 				new_CH = u;
// // 			} 
// // 			else if (countCloseCHs[u] == maxClose && maxE < E_current[u]) {
// // 				maxClose = countCloseCHs[u];
// // 				maxE = E_current[u];
// // 				new_CH = u;
// // 			}
// // 		}

// // 		/* else if (!A.empty()) {
// // 			double d_min = DBL_MAX;
// // 			for (int CH : A) {
// // 				vector<int> S_CH;
// // 				S_CH.push_back(CH);
// // 				growBallsWeightBounded(S_CH, 4*W);
// // 				for (int u : uncovered_nodes) {
// // 					if (clus_id[u] != -1) continue;
// // 					for (int v : graph.getAdjExceptSink(u)) {
// // 						if (clus_id[v] != -1 && distanceToCH[v] + distance(u,v) < d_min) {
// // 							d_min = distanceToCH[v] + distance(u,v);
// // 							new_CH = u;
// // 						}
// // 					}
// // 				}
// // 				if (d_min != DBL_MAX) break;
// // 			}
// // 			if (d_min == DBL_MAX) {
// // 				auto it3 = std::begin(uncovered_nodes);
// // 				auto r3 = rand() % uncovered_nodes.size();
// // 				std::advance(it3,r3);
// // 				new_CH = *it3;
// // 			}
// // 		} */

// // 		// Grow Ball
// // 		A.push_back(new_CH);
// // 		growBallsWeightBounded(A, W);
// // 		for (int i=0; i<N; i++) {
// // 			if (!isAliveSN(i)) continue;
// // 			if (clus_id[i] != -1) uncovered_nodes.erase(i);
// // 		}
// // 	}
// // 	// growBalls(A);
// // 	computeBallWeight();
// // 	double maxWeight = 0;
// // 	for (auto pair : ballWeight) {
// // 		if (pair.second > maxWeight) maxWeight = pair.second;
// // 	}routing_trace() << "maxWeight " << maxWeight << " W " << W;
// // 	routing_trace() << "epsilon " << maxWeight/W_total*A.size() - 1;

// // 	clus_id[self] = -1;
// // 	for (int l : A) clus_id[l] = -1;
// // 	return 0;
// // }


// void GAMBACBalanceNew::growBallsWeightBounded(vector<int> CHSet, double W){
// 	for (int u : graph.getNodesExceptSink()){
// 		distanceToCH[u] = DBL_MAX;
// 		clus_id[u] = -1;
// 		nextHop[u] = -1;
// 		clusterMembers[u].clear();
// 	}
// 	for (int u : CHSet) {
// 		distanceToCH[u] = 0;
// 		clusterMembers[u].clear();
// 		clus_id[u] = u;
// 	}

// 	dCompare =  &distanceToCH;
// 	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
// 	for (int l : CHSet) {
// 		queue.push(l);
// 	}
// 	unordered_set<int> removedSet;
// 	vector<double> T(N, 0);
// 	vector<int> level1_node(N, -1);
// 	for (int u : CHSet) {
// 		T[clus_id[u]] = dataVolume[u];
// 	}

// 	while (!queue.empty()) {
// 		int u = queue.top();
// 		queue.pop();
// 		trace() << "u " << u << " d[u] " << distanceToCH[u];
// 		if (removedSet.find(u) != removedSet.end()) continue;
// 		removedSet.insert(u);
// 		if (!isCH[u]) clusterMembers[clus_id[u]].push_back(u);
		
// 		for (int v : graph.getAdjExceptSink(u)) {
// 			if (!isAliveSN(v)) continue;
// 			if ((removedSet.find(v) != removedSet.end())) continue;
// 			double alt = distanceToCH[u] + graph.getLength(u,v);
// 			double T_new = T[clus_id[u]] + dataVolume[v];
// 			double T_level1 = 0;
// 			if (level1_node[u] != -1) T_level1 = T[level1_node[u]] + dataVolume[v];
			
// 			if (alt - distanceToCH[v] < -EPSILON && T_new <= W){
// 				T[clus_id[u]] = T_new;
// 				if (clus_id[v] != -1) T[clus_id[v]] -= dataVolume[v];
// 				distanceToCH[v] = alt;
// 				clus_id[v] = clus_id[u];
// 				nextHop[v] = u;
// 				queue.push(v);
// 			}
// 		}
// 	}
// }