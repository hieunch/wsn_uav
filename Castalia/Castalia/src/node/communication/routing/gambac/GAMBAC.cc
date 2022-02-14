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

	d0 = par("d0");
	gamma = par("gamma");
	maxCHperUAV = par("maxCHperUAV");
	W_opt = -1;
	W_start = 0;
	countSuccess = 0;
	countAdjusment = 0;
	debugRecruitProcess = par("debugRecruitProcess");
	epsilon = par("epsilon");
	if (isSink) GAMBACinit();
		
	setTimer(START_ROUND, 50);
}

void GAMBAC::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// weights[self] = pkt->getByteLength();
		string dst(destination);
		GAMBACPacket *netPacket = new GAMBACPacket("GAMBAC routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setGAMBACPacketKind(GAMBAC_ROUTING_DATA_PACKET);
		netPacket->setSource(SELF_NETWORK_ADDRESS);
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
			string dst(netPacket->getDestination());
			if (dst.compare(SELF_NETWORK_ADDRESS) == 0){
				int sourceId = netPacket->getSourceId();
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
					trace1() << "Energy\tE_min\ti_min\tlevel\tconfig.clus_id[i_min]\tcalculateRxSize(i_min)\tweights[i_min]\tE_total/numNodes\ttotalConsumed\tmaxConsumed\tmaxTxSize\tconfig.A.size()\tdevE\tavgConsumed\tobjective value\tE0_min\ttime_elapse\ttotalCollected\tmaxLengthRatio";
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
			totalConsumed = 0;
			maxConsumed = 0;
			maxTxSize = 0;
			auto start = std::chrono::steady_clock::now();
			totalCollected = 0;
			mainAlg();
			logConfig();
			auto end = std::chrono::steady_clock::now();
			time_elapse = end - start;
			break;
		}
		
		case SEND_DATA:{
			trace() << "SEND_DATA";
			RoutingPacket *dataPacket = check_and_cast <RoutingPacket*>(pkt);
			dataPacket->setByteLength(dataPacketSize);
			dataPacket->setKind(NETWORK_LAYER_PACKET);
			dataPacket->setSource(self);
			dataPacket->setDestination(config.clus_id[self]);
  			dataPacket->setTTL(1000);
			sendData(dataPacket);
			// processBufferedPacket();
			break;
		}
		case END_ROUND:{
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
				// trace1() << "E " << i << " " << E_i << " " << config.clus_id[i] << " " << config.nextHop[i] << " " << rxSizes[i]/1000 << " " << energyConsumeds[i];
			}
			trace1() << ss.str();

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
			// trace1() << "E_min " << E_min;
			trace1() << "Energy\t" << E_min << "\t" << i_min << "\t" << level << "\t" << config.clus_id[i_min] << "\t" << calculateRxSize(i_min) << "\t" << weights[i_min] << "\t" << E_total/numNodes << "\t" << totalConsumed << "\t" << maxConsumed << "\t" << maxTxSize << "\t" << config.A.size() << "\t" << devE << "\t" << avgConsumed << "\t" << objVal  << "\t" << E0_min << "\t" << time_elapse.count() << "\t" << totalCollected << "\t" << maxLengthRatio;
			// for (int u : outerSet_0) debugPoint(location(u), "blue");
			// for (int u : innerSet_0) debugPoint(location(u), "yellow");
			// for (int u : A2_0) debugPoint(location(u), "green");
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
	for (int i=0; i<numNodes; i++) W_total += weights[i];
	W_start = 4*W_total/numUAVs;
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
		clearData();
		
		W = (1 + epsilon) * W_total / k;
		
		trace1() << "k " << k << " k_start " << k_start << " k_end " << k_end;
		trace1() << "buildDFT";
		int loop_count = buildDFT(k);
		trace1() << "buildTrajectories " << A.size();
		buildTrajectories();
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


		if (lengthViolated) {
			// Increase W
			retry += 1;
			if (retry >= 5) {
				W_start = W;
				retry = 0;
			}
			else continue;
		} else {
			if (A.size() > config.A.size() && loop_count <= 50){
				trace1() << "save config";
				config.save(A, clus_id, next, trajectories);
				isSaved = true;
				k_start = k;
				// if (ratio > 0.95) break;
			}
			else {
				retry += 1;
				if (retry >= 5) {
					if (!isSaved) {
						config.save(A, clus_id, next, trajectories);
						epsilon_saved = epsilon0;
						isSaved = true;
					}
					W_end = W;
					retry = 0;
				}
				continue;
			}
			
			W_end = W;
			retry = 0;
		}

		W = W*ratioToLmax;
		trace() << "New value of W: " << W << " ratioToLmax " << ratioToLmax;
		if (W - W_end <= 10000) break;
		
		
		if ((W_start-W_end <= 10000) && !isSaved) {
			W_start = 4*W_total/numUAVs;
			W_end = 1000;
			W_opt = -1;
		}

	} while (W_start-W_end>10000);

	
	A = config.A;
	stringstream ssA;
	for(int l : A) {
		ssA << l << " ";
	}
	trace1() << "CH list: " << ss1.str();

	///////////////////////////////////////////////////////////

	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;

		stringstream ss;
		for(int l : T) {
			ss << l << " ";
		}
		trace1() << ss.str();
	}
	trace1() << "maxLength " << maxLength;
	maxLengthRatio = maxLength / L_max;
	trace1() << "END mainAlg";
}


void GAMBAC::GAMBACinit() {
	trace() << "GAMBACinit";
	
	N = numNodes;
	graph.init(numNodes, self, d0);
	trajectories.resize(numUAVs);
	d2CH.resize(N, 0.);
	clus_id.resize(N, -1);
	nextHop.resize(N, -1);
	isCH.resize(N, false);
	representSet.resize(N);
	w_max.resize(N);
	E_tmp = vector<double>(numNodes, 0);
}

void GAMBAC::reset() {
	
	trajectories = vector<vector<int>>(numUAVs);

	A.clear();
	d2CH = vector<double>(N, 0.);
	clus_id = vector<int>(N, -1);
	nextHop = vector<int>(N, -1);
	isCH = vector<bool>(N, false);	
	representSet = vector<list<int>>(N);
	w_max = vector<double>(N, 0);
}

vector<int> GAMBAC::randomInitCHSet(vector<int>W, double s) {
	int nodeW = Cand.size();
	if (nodeW > 0) {
		if (nodeW < s) {
			for (int node : W) {
				isCH[node] = true;
			}
			return W;
		} else {
			double prob_net = s / nodeW;
			vector<int> new_W;
			for (int node : W) {
				if (isCH[node]) continue;
				double random_float = uniform(0,1);
				if (random_float < prob_net) {
					new_Cand.push_back(node);
					isCH[node] = true;
				}
			}
			if (new_Cand.empty()) return randomInitCHSet(W, s);
			else {
				return new_W;
			}
		}
	}
	return W;
}

void GAMBAC::growBalls(vector<int> CHSet){
	for (int u : CHSet) {
		d2CH[u] = 0;
		representSet[u].clear();
		clus_id[u] = u;
	}

	dCompare =  &d2CH;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : CHSet) {
		queue.push(l);
	}
	unordered_set<int> removedSet;

	while (!queue.empty()) {
		int u = queue.top();
		queue.pop();
		trace() << "u " << u << " d[u] " << d2CH[u];
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		if (!isCH[u]) representSet[clus_id[u]].push_back(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = d2CH[u] + graph.getLength(u,v);
			
			if (alt - d2CH[v] < -EPSILON){
				d2CH[v] = alt;
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
				trace() << l << " " << ballWeight[l];
			}
			trace1() << "growBall ERROR " << ss.str();
			for (int v=0; v<numNodes; v++) {
				trace1() << v << " " << clus_id[v];
			}
		}
	}
	computeBallWeight();
}

vector<int> GAMBAC::getOuterSet(){
	for (int i=0; i<1; i++) trace() << "getOuterSet A.size " << A.size();
	int count = 1;
	outerSet.clear();
	innerSet.clear();
	vector<int> returnlist;
	for (int CH : A){
		for (int i=0; i<1; i++) trace() << "CH " << CH;
		double weight = ballWeight.find(CH)->second;

		if (weight > W) {
			weight = 0;
			dCompare =  &d2CH;
			priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
			for (int u : representSet[CH]) queue.push(u);
			while (weight < W){
				int u = queue.top();
				queue.pop();
				if (debugRecruitProcess) for (int i=0; i<1; i++) trace1() << "CH " << CH << " inner " << u << " d2CH " << d2CH[u] << " weight " << weight;
				innerSet.push_back(u);
				weight += weights[u];
			}
			while (!queue.empty()) {
				int u = queue.top();
				if (debugRecruitProcess) for (int i=0; i<1; i++) trace1() << "CH " << CH << " fringe " << u << " d2CH " << d2CH[u];
				queue.pop();
				outerSet.push_back(u);
			}
		}
	}
	return returnlist.empty() ? outerSet : returnlist;
}

vector<int> GAMBAC::samplingCH(vector<int> W, double b) {
	for (int i=0; i<0; i++) trace1() << "samplingCH";
	int maxTrial = 3;
	vector<int> new_W;
	while (!Cand.empty()){
		double Etotal = 0;
		for (int u : W) {
			Etotal += E_tmp[u];
		}
		double f = (double)rand() / RAND_MAX * Etotal;
		int nextLandmark = 0;
		for (int u : W) {
			f -= getResMgrModule(u)->getRemainingEnergy();
			if (f <= 0) {
				nextLandmark = u;
				break;
			}
		}
		if (isCH[nextLandmark]) {
			Cand.erase(std::remove(Cand.begin(), Cand.end(), nextLandmark), Cand.end());
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
			newBallSize += weights[v];
			if (newBallSize > w_max[nextLandmark]) break;
			if (std::find(Cand.begin(), Cand.end(), v) != Cand.end()){
				Cand.erase(std::remove(Cand.begin(), Cand.end(), v), Cand.end());
				intersectBall.push_back(v);
				intersectBallSize += weights[v];
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
		if ((maxTrial == 0) || (intersectBallSize>w_min)){//new_Cand.empty() &&
			new_Cand.push_back(nextLandmark);
			isCH[nextLandmark] = true;
			maxTrial = 3;
		}
		else{
			for (int v : intersectBall) Cand.push_back(v);
			if (maxTrial>0){
				maxTrial--;
				continue;
			}
			else break;
		}
	}
	return new_W;
}

void GAMBAC::computeBallWeight(){
	maxBallWeight = 0;
	minBallWeight = INT64_MAX;
	for (int u : graph.getNodesExceptSink()){
		representSet[clus_id[u]].push_back(u);
	}

	double tempBallWeight[A.size()];
	for (int i=0; i<A.size(); i++) tempBallWeight[i] = 0;
	int size = 0;
	for (int i=0; i<A.size(); i++){
		size += representSet[A[i]].size();
		for (int u : representSet[A[i]]) {
			tempBallWeight[i] += weights[u];
		}
	}
	ballWeight.clear();
	for (int i=0; i<A.size(); i++){
		ballWeight.insert(pair<int,double>(A[i], tempBallWeight[i]));
	}
}

void GAMBAC::recruitNewCHs(){
	for (int i=0; i<0; i++) trace1() << "recruitNewCHs";;
	vector<int> W;
	if (Cand.empty()){
		vector<int> outerSet = getOuterSet();
		
		for (int w : outerSet){
			auto P = GlobalLocationService::getLocation(w);
			Cand.push_back(w);
		};
	}
	A_recruited = samplingCH(Cand, 0);
	if (!A_recruited.empty()) A.insert(A.end(), A_recruited.begin(), A_recruited.end());
	clearData();
	growBalls(A);
}

void GAMBAC::clearData(){
	for (int u : graph.getNodesExceptSink()){
		if (isCH[u]){
			d2CH[u] = 0;
		}
		else {
			d2CH[u] = DBL_MAX;
		}
		clus_id[u] = -1;
		nextHop[u] = -1;
		representSet[u].clear();
	}
}

int GAMBAC::buildDFT(int k){
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

	int k_max;
	vector<double> w_max_ordered (w_max);
	sort(w_max_ordered.begin(), w_max_ordered.end(), greater<double>());
	double W_total_tmp = W_total;
	trace() << "W_total " << W_total;
	for (int i=0; i<N; i++) {
		trace() << "w_max_ordered " << i << " " << w_max_ordered[i];
		W_total_tmp -= w_max_ordered[i];
		if (W_total_tmp < 0) {
			k_max = i+1;
			break;
		}
	}
	
	k_max = (int) ((1 + epsilon) * W_total / W);
	W = (1 + epsilon) * W_total / k;
	
	int count = 0;
	clearData();
	vector<int> allSNs;
	for (int i=0; i<N; i++) allSNs.push_back(i);
	while (A.empty()) {
		A = randomInitCHSet(allSNs, 8);
	}
	growBalls(A);
	bool isBalance;
	do {
		int Asize = A.size();
		recruitNewCHs();

		isBalance = false;
		double W_new = min(W, (1 + epsilon) * W_total / A.size());
		double maxWeight = 0;
		double totalWeight = 0;
		for (auto pair : ballWeight) {
			if (maxWeight < pair.second) {
				maxWeight = pair.second;
			}
		}
		if (maxWeight > W_new) isBalance = true;

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
				int removed2CH;
				for (int l : A) {
					f -= 1./tempBallWeight[l];
					if (f <= 0) {
						removed2CH = l;
						break;
					}
				}
				A.erase(std::remove(A.begin(), A.end(), removed2CH), A.end());
				isCH[removed2CH] = false;
			}
		}
		clearData();
		growBalls(A);

		fl = false;
		W_new = min(W, (1 + epsilon) * W_total / A.size());
		maxWeight = 0;
		totalWeight = 0;
		for (auto pair : ballWeight) {
			if (maxWeight < pair.second) {
				maxWeight = pair.second;
			}
		}
		if (maxWeight > W_new) fl = true;

		if (count++ > 50) {
			break;
		}

		countAdjusment++;
		nloop = count;
	} while (fl);

	double maxWeight = 0;
	for (auto pair : ballWeight) {
		if (pair.second > maxWeight) maxWeight = pair.second;
	}
	trace() << "epsilon " << maxWeight/W_total*A.size();
	epsilon0 = maxWeight/W_total*A.size();
	// updateCentList();
	clus_id[self] = -1;
	for (int l : A) clus_id[l] = -1;
	for (int i=0; i<1; i++) trace() << "A size " << A.size();
	trace() << "W " << W;
	trace() << "k_max " << k_max;
	stringstream ss;
	for(int l : A) {
		// ss << l << " ";
		trace() << l << " " << ballWeight[l];
	}
	// trace() << ss.str();
	return count;
}

void GAMBAC::buildTrajectories(){
	for (int i=0; i<numUAVs; i++) trajectories[i].clear();
	if (A.size() < numUAVs) {
		for (int i=0; i<A.size(); i++) {
			trajectories[i].push_back(self);
			trajectories[i].push_back(A[i]);
			trajectories[i].push_back(self);
		}
	}
	else {
		for (int i=0; i<10; i++) trace() << "buildTrajectories";
		trajectories = Vrptw::call(*this, sinkId, A, numUAVs, L_max);
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
