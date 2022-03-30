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

#include "GAMBAC.h"

Define_Module(GAMBAC);

void GAMBAC::startup()
{	
	/*--- The .ned file's parameters ---*/
	applicationID = par("applicationID").stringValue(); 	

	neighborRange = par("neighborRange");
	gamma = par("gamma");
	maxCHperUAV = par("maxCHperUAV");
	W_opt = -1;
	W_start = 0;
	countSuccess = 0;
	countAdjusment = 0;
	debugRecruitProcess = par("debugRecruitProcess");
	dataPacketSize = par("dataPacketSize");
	epsilon = par("epsilon");
	if (isSink) GAMBACinit();
		
	setTimer(START_ROUND, 50);
}

void GAMBAC::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// amountData[self] = pkt->getByteLength();
		string dst(destination);
		GAMBACPacket *netPacket = new GAMBACPacket("GAMBAC routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setGAMBACPacketKind(GAMBAC_ROUTING_DATA_PACKET);
		netPacket->setSourceAddress(SELF_NETWORK_ADDRESS);
		encapsulatePacket(netPacket, pkt);
		bufferAggregate.push_back(*netPacket);
	}	
}

void GAMBAC::fromMacLayer(cPacket *pkt, int macAddress, double rssi, double lqi){
	GAMBACPacket *netPacket = dynamic_cast <GAMBACPacket*>(pkt);

	if (!netPacket)
		return;

	switch (netPacket->getGAMBACPacketKind()) {

		case GAMBAC_ROUTING_DATA_PACKET:{
			string dst(netPacket->getDestinationAddress());
			if (dst.compare(SELF_NETWORK_ADDRESS) == 0){
				int sourceId = netPacket->getSource();
				if (isSink) {
					toApplicationLayer(decapsulatePacket(netPacket));
				} else {
					bufferAggregate.push_back(*netPacket);	
				}
			}
			break;
		}
	}
}

void GAMBAC::timerFiredCallback(int index)
{
	switch (index) {
		
		case START_ROUND:{
			// for (int i=0; i<5; i++) trace() << "START_ROUND";
			if (roundNumber == 0) {
				resMgrModule->resetBattery();
				if (isSink)
					routing_trace() << "Energy\tE_min\ti_min\tlevel\tconfig.clus_id[i_min]\tcalculateRxSize(i_min)\tamountData[i_min]\tE_total/numNodes\ttotalConsumed\tmaxConsumed\tmaxTxSize\tconfig.A.size()\tdevE\tavgConsumed\tobjective value\tE0_min\ttime_elapse\ttotalCollected\tmaxLengthRatio";
			}
			
			if (isSink) {
				trace() << "START_ROUND";
				setTimer(START_MAINALG, 1);
				setTimer(END_ROUND, 3);

				E_min = DBL_MAX;
				for (int i=0; i<numNodes; i++) {
					if (i == self) continue;
					double E_i = getResMgrModule(i)->getRemainingEnergy();
					if (E_i < E_min) {
						E_min = E_i;
					}
				}
				E0_min = E_min;
			} else {
				setTimer(SEND_DATA, 2);
			}
			roundNumber++;
			setTimer(START_ROUND, roundLength);
			// for (int i=0; i<5; i++) trace() << "setTimer " << (roundLength + simTime());
			break;
		}
		case START_MAINALG:{	
			uncollectedSet.clear();
			collectedSet.clear();
			for (int i=0; i<N; i++) if (i != sinkId) uncollectedSet.insert(i);
			totalConsumed = 0;
			maxConsumed = 0;
			maxTxSize = 0;
			energyConsumeds = vector<double>(N, 0);
			rxSizes = vector<double>(N, 0);
			auto start = std::chrono::steady_clock::now();
			totalCollected = 0;
			mainAlg();
			logConfig();
			auto end = std::chrono::steady_clock::now();
			time_elapse = end - start;
			break;
		}
		
		case SEND_DATA:{
			// routing_trace() << "SEND_DATA";
			RoutingPacket *dataPacket = new GAMBACPacket("GAMBAC data packet", NETWORK_LAYER_PACKET);
			dataPacket->setByteLength(amountData[self]);
			dataPacket->setKind(NETWORK_LAYER_PACKET);
			dataPacket->setSource(self);
			dataPacket->setSourceAddress(SELF_NETWORK_ADDRESS);
			dataPacket->setDestination(config.clus_id[self]);
			dataPacket->setDestinationAddress(to_string(config.clus_id[self]).c_str());
  			dataPacket->setTTL(1000);
			sendData(dataPacket);
			// processBufferedPacket();
			break;
		}
		case END_ROUND:{
			// routing_trace() << "END_ROUND";
			stringstream ss_uncollected;
			for (int u : uncollectedSet) ss_uncollected << u << " ";
			routing_trace() << "uncollected " << ss_uncollected.str();
			routing_trace() << "collected " << collectedSet.size();

			int i_min;
			E_min = DBL_MAX;
			double E_total = 0;
			stringstream ss;
			for (int i=0; i<numNodes; i++) {
				if (i == self) continue;
				double E_i = getResMgrModule(i)->getRemainingEnergy();
				if (E_i < E_min) {
					E_min = E_i;
					i_min = i;
				}
				E_total += E_i;
				ss << E_i << " ";
			}
			routing_trace() << ss.str();

			int level = 0;
			int current = i_min;
			while (config.clus_id[current] != -1) {
				level++;
				current = config.nextHop[current];
			}
			double E_avg = E_total/(numNodes-1);
			double devE = 0;
			for (int i=0; i<numNodes; i++) {
				if (i == self) continue;
				double E_i = getResMgrModule(i)->getRemainingEnergy();
				devE += (E_i - E_avg) * (E_i - E_avg);
			}
			devE = devE/(numNodes-1);
			devE = sqrt(devE);
			double avgConsumed = 0;
			for (int i=0; i<numNodes; i++) {
				if (i == self) continue;
				avgConsumed += energyConsumeds[i];
			}
			avgConsumed = avgConsumed/(N-1);
			double objVal = -E_min/E0_min/2 + 1000*avgConsumed/1.8937;
			// routing_trace() << "E_min " << E_min;
			routing_trace() << "Energy\t" << E_min << "\t" << i_min << "\t" << level << "\t" << config.clus_id[i_min] << "\t" << calculateRxSize(i_min) << "\t" << amountData[i_min] << "\t" << E_total/numNodes << "\t" << totalConsumed << "\t" << maxConsumed << "\t" << maxTxSize << "\t" << config.A.size() << "\t" << devE << "\t" << avgConsumed << "\t" << objVal  << "\t" << E0_min << "\t" << time_elapse.count() << "\t" << totalCollected << "\t" << maxLengthRatio;
			break;
		}
	}
}

void GAMBAC::processBufferedPacket()
{
		
}



void GAMBAC::mainAlg() {

	for (int u=0; u<numNodes; u++) E_tmp[u] = getResMgrModule(u)->getRemainingEnergy();
	config.A.clear();

	for (int i=0; i<10; i++) trace() << "mainAlg";
	W_total = 0;
	for (int i=0; i<numNodes; i++) W_total += amountData[i];
	W_start = W_total/numUAVs/2;
	W_end = 1000;

	W = (W_start+W_end)/2;

	bool lengthViolated;
	bool isSaved = false;
	int retry = 0;
	double ratioToLmax = 1;
	double W_saved;
	double maxLen = 0;
	
	do {
		reset();

		routing_trace() << "W " << W << " W_start " << W_start << " W_end " << W_end;
		routing_trace() << "selectCHsAndBuildDFT";
		int loop_count = selectCHsAndBuildDFT(W);
		routing_trace() << "buildTrajectories " << A.size();
		if (A == config.A) {
			trajectories = config.trajectories;
		}
		else buildTrajectories(true);
		lengthViolated = false;
		maxLen = 0;
			
		// verify trajectories length
		for (int t=0; t<numUAVs; t++) {
			double len = calculatePathLength(trajectories[t]);
			if (len > L_max) {
				lengthViolated = true;
			};
			if (len > maxLen) maxLen = len;
		}
		ratioToLmax = pow(maxLen / L_max, 2);
		routing_trace() << "maxLen " << maxLen << " ratioToLmax " << ratioToLmax;


		if (lengthViolated) {
			// Increase W
			retry += 1;
			if (retry >= 5) {
				W_end = W;
				W = W*ratioToLmax;
				retry = 0;
			}
			else continue;
		} else {
			if (A.size() > config.A.size() && loop_count <= 50){
				routing_trace() << "save config";
				config.save(A, clus_id, nextHop, trajectories);
				isSaved = true;
				W_start = W;
				W = W*ratioToLmax;
			}
			else {
				retry += 1;
				if (retry >= 5) {
					if (!isSaved) {
						config.save(A, clus_id, nextHop, trajectories);
						isSaved = true;
					}
					W_start = W;
					W = W*ratioToLmax;
					retry = 0;
				}
				continue;
			}
			
			retry = 0;
		}

		if (W_start - W <= 10000) break;
		
		
		if ((W_start-W_end <= 10000) && !isSaved) {
			W_start = W_total/numUAVs/2;
			W_end = 1000;
			W_opt = -1;
		}

	} while (W_start-W_end>10000);

	
	A = config.A;
	stringstream ssA;
	for(int l : A) {
		ssA << l << " ";
	}
	routing_trace() << "CH list: " << ssA.str();

	///////////////////////////////////////////////////////////

	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;

		stringstream ss;
		for(int l : T) {
			ss << l << " ";
		}
		routing_trace() << ss.str();
	}
	routing_trace() << "maxLength " << maxLength;
	maxLengthRatio = maxLength / L_max;
	routing_trace() << "END mainAlg";
}


void GAMBAC::GAMBACinit() {
	trace() << "GAMBACinit";
	
	N = numNodes;
	graph.init(numNodes, self, neighborRange);
	trajectories.resize(numUAVs);
	distanceToCH.resize(N, 0.);
	clus_id.resize(N, -1);
	nextHop.resize(N, -1);
	isCH.resize(N, false);
	clusterMembers.resize(N);
	w_max.resize(N);
	E_tmp = vector<double>(numNodes, 0);

	// routing_trace() << "neighborRange " << neighborRange;
	// for (int i=0; i<numNodes; i++) {
	// 	routing_trace() << "neighbor " << i << " " << graph.getAdjExceptSink(i).size();
	// }
}

void GAMBAC::reset() {
	
	trajectories = vector<vector<int>>(numUAVs);

	A.clear();
	distanceToCH = vector<double>(N, 0.);
	clus_id = vector<int>(N, -1);
	nextHop = vector<int>(N, -1);
	isCH = vector<bool>(N, false);	
	clusterMembers = vector<list<int>>(N);
	w_max = vector<double>(N, 0);
}

vector<int> GAMBAC::randomFromSet(vector<int> Candidates, double s) {
	int nodeW = Candidates.size();
	if (nodeW > 0) {
		if (nodeW < s) {
			for (int node : Candidates) {
				isCH[node] = true;
			}
			return Candidates;
		} else {
			double prob_net = s / nodeW;
			vector<int> selected_candidates;
			for (int node : Candidates) {
				if (isCH[node]) continue;
				double random_float = uniform(0,1);
				if (random_float < prob_net) {
					selected_candidates.push_back(node);
					isCH[node] = true;
				}
			}
			if (selected_candidates.empty()) return randomFromSet(Candidates, s);
			else {
				return selected_candidates;
			}
		}
	}
	return Candidates;
}

void GAMBAC::growBalls(vector<int> CHSet){

	for (int u : graph.getNodesExceptSink()){
		if (isCH[u]){
			distanceToCH[u] = 0;
		}
		else {
			distanceToCH[u] = DBL_MAX;
		}
		clus_id[u] = -1;
		nextHop[u] = -1;
		clusterMembers[u].clear();
	}
	
	for (int u : CHSet) {
		distanceToCH[u] = 0;
		clusterMembers[u].clear();
		clus_id[u] = u;
	}

	dCompare =  &distanceToCH;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : CHSet) {
		queue.push(l);
	}
	unordered_set<int> removedSet;

	while (!queue.empty()) {
		int u = queue.top();
		queue.pop();
		trace() << "u " << u << " d[u] " << distanceToCH[u];
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		if (!isCH[u]) clusterMembers[clus_id[u]].push_back(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = distanceToCH[u] + graph.getLength(u,v);
			
			if (alt - distanceToCH[v] < -EPSILON){
				distanceToCH[v] = alt;
				clus_id[v] = clus_id[u];
				nextHop[v] = u;
				queue.push(v);
			}
		}
	}
	int totalSize = 0;
	for (int u=0; u<numNodes; u++) {
		if (u ==sinkId) continue;
		if (clus_id[u] == -1) {
			stringstream ss;
			for(int l : A) {
				ss << l << " ";
			}
			routing_trace() << "growBall ERROR " << ss.str();
			for (int v=0; v<numNodes; v++) {
				routing_trace() << v << " " << clus_id[v];
			}
		}
	}
	computeBallWeight();
}

vector<int> GAMBAC::getOuterOversizePart(){
	outerSet.clear();
	innerSet.clear();
	vector<int> returnlist;
	for (int CH : A){
		double weight = ballWeight.find(CH)->second;

		if (weight > w_max[CH]) {
			weight = 0;
			dCompare =  &distanceToCH;
			priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
			for (int u : clusterMembers[CH]) queue.push(u);
			while (weight < w_max[CH]){
				int u = queue.top();
				queue.pop();
				if (debugRecruitProcess) for (int i=0; i<1; i++) routing_trace() << "CH " << CH << " inner " << u << " distanceToCH " << distanceToCH[u] << " weight " << weight;
				weight += amountData[u];
				if (weight >= w_max[CH]) break;
				innerSet.push_back(u);
			}
			while (!queue.empty()) {
				int u = queue.top();
				if (debugRecruitProcess) for (int i=0; i<1; i++) routing_trace() << "CH " << CH << " fringe " << u << " distanceToCH " << distanceToCH[u];
				queue.pop();
				outerSet.push_back(u);
			}
		}
	}
	return returnlist.empty() ? outerSet : returnlist;
}

vector<int> GAMBAC::samplingCH(vector<int> Candidates, double b) {
	for (int i=0; i<0; i++) routing_trace() << "samplingCH";
	int maxTrial = 3;
	vector<int> selected_candidates;
	while (!Candidates.empty()){
		int nextLandmark = Candidates[rand() % Candidates.size()];

		if (isCH[nextLandmark]) {
			Candidates.erase(std::remove(Candidates.begin(), Candidates.end(), nextLandmark), Candidates.end());
			continue;
		}

		vector<int> intersectBall;
		double newBallSize = 0;
		double intersectBallSize = 0;

		vector<double> d(N, DBL_MAX);
		d[nextLandmark] = 0;
		dCompare =  &d;
		priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
		queue.push(nextLandmark);
		unordered_set<int> removedSet;
		while (!queue.empty()){
			int v = queue.top();
			queue.pop();
			if (removedSet.find(v) != removedSet.end()) continue;
			removedSet.insert(v);
			newBallSize += amountData[v];
			if (newBallSize > w_max[nextLandmark]) break;
			if (std::find(Candidates.begin(), Candidates.end(), v) != Candidates.end()){
				Candidates.erase(std::remove(Candidates.begin(), Candidates.end(), v), Candidates.end());
				intersectBall.push_back(v);
				intersectBallSize += amountData[v];
			}
			for (int w : graph.getAdjExceptSink(v)){
				if ((w < N) && (removedSet.find(w) == removedSet.end())){
					double alt = d[v] + graph.getLength(v,w);
					if (alt < d[w]){
						d[w] = alt;
						queue.push(w);
					}
				}
			}
		}
		if ((maxTrial == 0) || (intersectBallSize>W/4)){//selected_candidates.empty() &&
			selected_candidates.push_back(nextLandmark);
			maxTrial = 3;
		}
		else{
			for (int v : intersectBall) Candidates.push_back(v);
			if (maxTrial>0){
				maxTrial--;
				continue;
			}
			else break;
		}
	}
	return selected_candidates;
}

void GAMBAC::computeBallWeight(){
	maxBallWeight = 0;
	minBallWeight = INT64_MAX;
	
	double tempBallWeight[A.size()];
	for (int i=0; i<A.size(); i++) tempBallWeight[i] = 0;
	int size = 0;
	for (int i=0; i<A.size(); i++){
		size += clusterMembers[A[i]].size();
		for (int u : clusterMembers[A[i]]) {
			tempBallWeight[i] += amountData[u];
		}
	}
	ballWeight.clear();
	for (int i=0; i<A.size(); i++){
		ballWeight.insert(pair<int,double>(A[i], tempBallWeight[i]));
	}
}

void GAMBAC::recruitNewCHs(){
	for (int i=0; i<0; i++) routing_trace() << "recruitNewCHs";;
	vector<int> Candidates = getOuterOversizePart();
	// routing_trace() << "Candidates size " << Candidates.size();
	vector<int> newCHs = samplingCH(Candidates, 0);
	if (!newCHs.empty()) A.insert(A.end(), newCHs.begin(), newCHs.end());
	for (int u : newCHs) {
		isCH[u] = true;
	}
}

int GAMBAC::selectCHsAndBuildDFT(double W){
	E_min = DBL_MAX;
	E_max = 0;
	for (int i=0; i<numNodes; i++) {
		if (i == sinkId) continue;
		if (E_tmp[i] > E_max) E_max = E_tmp[i];
		if (E_tmp[i] < E_min) E_min = E_tmp[i];
	}

	for (int i=0; i<N; i++) {
		if (i == sinkId) continue;
		w_max[i] = W;
	}
	
	int k_max = (int) ((1 + epsilon) * W_total / W);
	
	int count = 0;
	vector<int> allSNs;
	for (int i=0; i<N; i++) allSNs.push_back(i);
	while (A.empty()) {
		A = randomFromSet(allSNs, 8);
	}
	routing_trace() << "initial size " << A.size();
	growBalls(A);
	bool isBalance;
	do {
		int Asize = A.size();
		recruitNewCHs();
		growBalls(A);

		isBalance = true;
		double W_new = min(W, (1 + epsilon) * W_total / A.size());
		double maxWeight = 0;
		double totalWeight = 0;
		for (auto pair : ballWeight) {
			if (maxWeight < pair.second) {
				maxWeight = pair.second;
			}
		}
		if (maxWeight > W_new) isBalance = false;

		if (A.size() > k_max) {
			vector<double> tempBallWeight(N, 0);
			for (auto pair : ballWeight) {
				tempBallWeight[pair.first] = pair.second;
			}
			dCompare =  &tempBallWeight;
			priority_queue<int,vector<int>, decltype(&Comparebydistance)> Q(Comparebydistance);
			for (int l : A) Q.push(l);
			while (A.size() > k_max){
				double WeightTotal = 0;
				for (int l : A) {
					WeightTotal += 1./tempBallWeight[l];
				}
				double f = (double)rand() / RAND_MAX * WeightTotal;
				int u;
				for (int l : A) {
					f -= 1./tempBallWeight[l];
					if (f <= 0) {
						u = l;
						break;
					}
				}
				A.erase(std::remove(A.begin(), A.end(), u), A.end());
				isCH[u] = false;
			}
		}
		growBalls(A);

		isBalance = true;
		W_new = min(W, (1 + epsilon) * W_total / A.size());
		maxWeight = 0;
		totalWeight = 0;
		for (auto pair : ballWeight) {
			if (maxWeight < pair.second) {
				maxWeight = pair.second;
			}
			if (w_max[pair.first] < pair.second) {
				isBalance = false;
			}
		}
		routing_trace() << "loop " << count << " epsilon " << maxWeight/W_total*A.size() - 1;

		if (count++ > 50) {
			break;
		}

		countAdjusment++;
		nloop = count;
	} while (!isBalance);

	double maxWeight = 0;
	for (auto pair : ballWeight) {
		if (pair.second > maxWeight) maxWeight = pair.second;
	}
	routing_trace() << "epsilon " << maxWeight/W_total*A.size() - 1 << " count " << count;
	clus_id[self] = -1;
	for (int l : A) clus_id[l] = -1;
	for (int i=0; i<1; i++) trace() << "A size " << A.size();
	trace() << "W " << W;
	trace() << "k_max " << k_max;
	return count;
}

void GAMBAC::buildTrajectories(){
	buildTrajectories(false);
}

void GAMBAC::buildTrajectories(bool isBreak){
	for (int i=0; i<numUAVs; i++) trajectories[i].clear();
	if (A.size() < numUAVs) {
		for (int i=0; i<A.size(); i++) {
			trajectories[i].push_back(self);
			trajectories[i].push_back(A[i]);
			trajectories[i].push_back(self);
		}
	}
	else {
		// trajectories = Vrptw::call(*this, sinkId, A, numUAVs, L_max);
		auto initialTrajectories = buildInitialTrajectories();
		trajectories = Vrptw::call(*this, sinkId, A, numUAVs, L_max, isBreak, initialTrajectories);
		while (trajectories.size() < numUAVs) {
			trajectories.push_back(vector<int>());
		}
	}

	double maxLength = 0;
	unordered_set<int> CHs;
	CHs.insert(A.begin(), A.end());
	for (int k=0; k<numUAVs; k++) {
		double length = calculatePathLength(trajectories[k]);
		if (length > maxLength) maxLength = length;
		trace() << "trajectory " << k << " length " << length;
		stringstream ss;
		for(int i : trajectories[k]) {
			ss << i << " ";
			CHs.erase(i);
		}
		trace() << ss.str();
	}
	trace() << "maxLength " << maxLength;
	for (int i : CHs) trace() << i << " is unvisited!!!";

	for (int k=0; k<trajectories.size(); k++) {
		if (trajectories[k].empty()) {
			trajectories[k].push_back(sinkId);
			trajectories[k].push_back(sinkId);
		}
	}
};

vector<vector<int>> GAMBAC::buildInitialTrajectories(){
	vector<int> A_tmp = A;
	vector<int> TSP_tour = TSP(A_tmp);
	vector<vector<int>> new_trajectories;

	routing_trace() << "TSPLength = " << calculatePathLength(TSP_tour);
	stringstream ss_tsp;
	for (int l : TSP_tour) {
		ss_tsp << l << " ";
	}
	routing_trace() << "TSP tour: " << ss_tsp.str();
	double minMaxLength = DBL_MAX;
	for (int offset=0; offset<TSP_tour.size(); offset++) {
		double L_max_last = L_max;
		double maxLengthLast;
		double maxLength = DBL_MAX;
		vector<vector<int>> tmp_trajectories;
		do {
			maxLengthLast = maxLength;
			maxLength = 0;
			tmp_trajectories = vector<vector<int>>(numUAVs);
			for (auto &trajectory : tmp_trajectories) trajectory.push_back(sinkId);
			int cur_trajectory_id = 0;
			double curLength = 0;
			double totalLength = 0;
			for (int i=0; i<TSP_tour.size(); i++) {
				int node = TSP_tour[(i+offset)%TSP_tour.size()];
				vector<int> &cur_trajectory = tmp_trajectories[cur_trajectory_id];
				int lastNode = cur_trajectory[cur_trajectory.size()-1];
				double curLengthLast = curLength;
				curLength += distance(lastNode, node) + distance(node, sinkId) - distance(lastNode, sinkId);
				if (curLength > L_max_last) {
					if (cur_trajectory_id < numUAVs-1) {
						cur_trajectory.push_back(sinkId);
						totalLength += curLengthLast;
						cur_trajectory_id++;
						vector<int> &next_trajectory = tmp_trajectories[cur_trajectory_id];
						next_trajectory.push_back(node);
						curLength = distance(sinkId, node)*2;
						if (curLength > maxLength) maxLength = curLength;
					}
					else {
						cur_trajectory.push_back(node);
						if (curLength > maxLength) maxLength = curLength;
					}
				}
				else {
					cur_trajectory.push_back(node);
					if (curLength > maxLength) maxLength = curLength;
				}
			}
			tmp_trajectories[cur_trajectory_id].push_back(sinkId);
			totalLength += curLength;
			L_max_last = totalLength / numUAVs;
			if (maxLength < minMaxLength) {
				minMaxLength = maxLength;
				new_trajectories = tmp_trajectories;
			}
		} while (maxLength < maxLengthLast);
	}

	routing_trace() << "minMaxLength = " << minMaxLength;

	int jj = 1;
	for (auto trajectory : new_trajectories) {
		stringstream ss_tra;
		for (int u : trajectory) ss_tra << u << " ";
		routing_trace() << "Tour " << jj << ": " << ss_tra.str() << " - Length: " << calculatePathLength(trajectory);
		jj++;
	}
	vector<vector<int>> tours(numUAVs);
	vector<int> A_new;
	int matchingId = 1;
	for (int i=0; i<numUAVs; i++) {
		for (int u : new_trajectories[i]) {
			if (u != sinkId) {
				A_new.push_back(u);
				tours[i].push_back(matchingId);
				matchingId++;
			}
		}
	}
	A = A_new;
	return tours;
}

vector<int> GAMBAC::TSP(vector<int> AA) {
	vector<int> TSP_tour;
	vector<Point> sites;
	for (int l : AA) {
		Point p = location(l);
		p.id_ = l;
		sites.push_back(p);
	}
	
	while (!sites.empty()) {
		vector<Point> convexHull = G::convexHull(sites);
		if (convexHull.empty()) {
			convexHull = sites;
		}
		for (Point p : convexHull) {
			sites.erase(std::remove(sites.begin(), sites.end(), p), sites.end());
		}
		if (TSP_tour.empty()) {
			for (Point p : convexHull) {
				TSP_tour.push_back(p.id_);
			}
		}
		else {
			while (!convexHull.empty()) {
				double minIncreased = DBL_MAX;
				int i_min = -1;
				int pos = -1;
				for (int i=0; i<convexHull.size(); i++) {
					Point p = convexHull[i];
					for (int j=0; j<TSP_tour.size(); j++) {
						Point A = location(TSP_tour[j]);
						Point B = location(TSP_tour[(j+1)%TSP_tour.size()]);
						double lengthIncreased = G::distance(A, p) + G::distance(p, B) - G::distance(A, B);
						if (lengthIncreased < minIncreased) {
							minIncreased = lengthIncreased;
							i_min = i;
							pos = (j+1)%TSP_tour.size();
						}
					}
				}

				Point insertedNode = convexHull[i_min];
				convexHull.erase(convexHull.begin() + i_min);
				TSP_tour.insert(TSP_tour.begin() + pos, insertedNode.id_);
			}
		}
	}
	return TSP_tour;
}