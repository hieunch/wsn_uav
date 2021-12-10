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

#include "konstant2019.h"

Define_Module(konstant2019);

void konstant2019::startup()
{	
	/*--- The .ned file's parameters ---*/
	applicationID = par("applicationID").stringValue(); 	

	d0 = par("d0");
	gamma = par("gamma");
	Wt_opt = -1;
	Wt_max = 0;
	if (isSink) GPinit();
		
	setTimer(START_ROUND, 50);
}

void konstant2019::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// weights[self] = pkt->getByteLength();
		string dst(destination);
		konstant2019Packet *netPacket = new konstant2019Packet("GP routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setKonstant2019PacketKind(GP_ROUTING_DATA_PACKET);
		netPacket->setSource(SELF_NETWORK_ADDRESS);
		encapsulatePacket(netPacket, pkt);
		bufferAggregate.push_back(*netPacket);
	}	
}

void konstant2019::fromMacLayer(cPacket *pkt, int macAddress, double rssi, double lqi){
	konstant2019Packet *netPacket = dynamic_cast <konstant2019Packet*>(pkt);

	if (!netPacket)
		return;

	switch (netPacket->getKonstant2019PacketKind()) {

		case GP_ROUTING_DATA_PACKET:{
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

void konstant2019::timerFiredCallback(int index)
{
	switch (index) {
		
		case START_ROUND:{
			// for (int i=0; i<5; i++) trace() << "START_ROUND";
			if (roundNumber == 0) {
				resMgrModule->resetBattery();
				if (isSink)
					trace1() << "Energy\tE_min\ti_min\tlevel\tconfig.cent[i_min]\tcalculateRxSize(i_min)\tweights[i_min]\tE_total/numNodes\ttotalConsumed\tmaxConsumed\tmaxTxSize\tconfig.A.size()\tdevE\tavgConsumed\tobjective value\tE0_min\ttime_elapse\ttotalCollected\tmaxLengthRatio";
			}
			
			if (isSink) {
				trace() << "START_ROUND";
				setTimer(START_CLUSTERING, 1);
				setTimer(END_ROUND, 3);

				E_min = DBL_MAX;
				for (int i=0; i<numNodes; i++) {
					if (i == self) continue;
					double E_i = getResMgrModule(i)->getRemainingEnergy();
					if (E_i < E_min) {
						E_min = E_i;
					}
				}
			} else {
				setTimer(START_SLOT, 2);
			}
			roundNumber++;
			setTimer(START_ROUND, roundLength);
			// for (int i=0; i<5; i++) trace() << "setTimer " << (roundLength + simTime());
			break;
		}
		case START_CLUSTERING:{	
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
		
		case START_SLOT:{
			sendAggregate();
			// trace() << "Send aggregated packet to " << -1;
			// processBufferedPacket();
			break;
		}
		case END_ROUND:{	
			int i_min;	
			double E0_min = E_min;
			E_min = DBL_MAX;
			double E_total = 0;
			for (int i=0; i<numNodes; i++) {
				if (i == self) continue;
				double E_i = getResMgrModule(i)->getRemainingEnergy();
				if (E_i < E_min) {
					E_min = E_i;
					i_min = i;
				}
				E_total += E_i;
				// trace1() << "E " << i << " " << E_i << " " << config.cent[i] << " " << config.next[i] << " " << rxSizes[i]/1000 << " " << energyConsumeds[i];
			}
			int level = 0;
			int current = i_min;
			while (config.cent[current] != -1) {
				level++;
				current = config.cent[current];
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
			trace1() << "Energy\t" << E_min << "\t" << i_min << "\t" << level << "\t" << config.cent[i_min] << "\t" << calculateRxSize(i_min) << "\t" << weights[i_min] << "\t" << E_total/numNodes << "\t" << totalConsumed << "\t" << maxConsumed << "\t" << maxTxSize << "\t" << config.A.size() << "\t" << devE << "\t" << avgConsumed << "\t" << objVal  << "\t" << E0_min << "\t" << time_elapse.count() << "\t" << totalCollected << "\t" << maxLengthRatio;
			for (int u : fringeSet_0) debugPoint(location(u), "blue");
			for (int u : innerSet_0) debugPoint(location(u), "yellow");
			for (int u : A2_0) debugPoint(location(u), "green");
			break;
		}
	}
}

void konstant2019::processBufferedPacket()
{
		
}



void konstant2019::mainAlg() {
	for (int i=0; i<1; i++) trace1() << "mainAlg";
	reset();
	clearData();
	findEnergyEfficientSolution();

	for (int k=0; k<numUAVs; k++) {
		// trace1() << "trajectory " << k << " " << calculatePathLength(config.trajectories[k]);
		stringstream ss;
		for(int l : config.trajectories[k]) {
			ss << l << " ";
		}
		// trace1() << ss.str();
	}
	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;
	}
	trace1() << "maxLength " << maxLength;
	maxLengthRatio = maxLength / L_max;
	// trace1() << "A size " << config.A.size();
}


void konstant2019::GPinit() {
	// for (int i=0; i<10; i++) trace() << "GPinit";
	
	N = numNodes;
	graph.init(numNodes, self, d0);
	trajectories.resize(numUAVs);

	dLandmark.resize(N, 0.);
	cent.resize(N, -1);
	next.resize(N, -1);
	centList.resize(N);
	isLandmark.resize(N, false);
	representSet.resize(N);
	w_max.resize(N);
	E0.resize(N);
	for (int u=0; u<N; u++) {
		E0[u] = getResMgrModule(u)->getInitialEnergy() + 0.04;
	}
}

void konstant2019::reset() {
	
	trajectories = vector<vector<int>>(numUAVs);

	A.clear();
	dLandmark = vector<double>(N, 0.);
	cent = vector<int>(N, -1);
	next = vector<int>(N, -1);
	centList = vector<list<int>>(N);
	isLandmark = vector<bool>(N, false);
	representSet = vector<list<int>>(N);
	w_max = vector<double>(N, 0);
}

vector<int> konstant2019::TZ_sample(vector<int>W, double s) {
	// for (int i=0; i<5; i++) trace() << "TZ_sample";
	int nodeW = W.size();
	if (nodeW > 0) {
		if (nodeW < s) {
			for (int node : W) {
				isLandmark[node] = true;
			}
			return W;
		} else {
			double prob_net = s / nodeW;
			vector<int> new_W;
			for (int node : W) {
				if (isLandmark[node]) continue;
				double random_float = uniform(0,1);
				if (random_float < prob_net) {
					new_W.push_back(node);
					isLandmark[node] = true;
				}
			}
			if (new_W.empty()) return TZ_sample(W, s);
			else {
				return new_W;
			}
		}
	}
	return W;

}

void konstant2019::growBallsKonstant(vector<int> landmarkSet){
	// for (int i=0; i<20; i++) trace() << "growBalls Asize " << A.size();
	vector<double> vals(numNodes, 0);
	vector<double> d(numNodes, DBL_MAX);
	for (int u : landmarkSet) {
		d[u] = 0;
		representSet[u].clear();
		cent[u] = u;
		next[u] = -1;
	}

	unordered_set<int> removedSet;
	removedSet.insert(landmarkSet.begin(), landmarkSet.end());

	for (int v=0; v<numNodes; v++){
		if (removedSet.find(v) != removedSet.end()) continue;
		for (int u : landmarkSet){
			double val = - getResMgrModule(u)->getRemainingEnergy() / (d[u] + distance(u,v)) / (d[u] + distance(u,v));
			if (val < vals[v]) {
				vals[v] = val;
				cent[v] = u;
				next[v] = u;
			}
		}
	}

	dCompare =  &vals;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : landmarkSet) {
		queue.push(l);
	}

	while (!queue.empty()) {
		int u = queue.top();
		queue.pop();
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		d[u] = d[next[u]] + distance(u, next[u]);
		// if (!isLandmark[u]) representSet[cent[u]].push_back(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = - getResMgrModule(u)->getRemainingEnergy() / (d[u] + distance(u,v)) / (d[u] + distance(u,v));
			
			if (alt - vals[v] < -EPSILON){
				vals[v] = alt;
				cent[v] = cent[u];
				next[v] = u;
				queue.push(v);
			}
		}
	}
}

void konstant2019::growBalls(vector<int> landmarkSet){
	// for (int i=0; i<20; i++) trace() << "growBalls Asize " << A.size();
	for (int u : landmarkSet) {
		dLandmark[u] = 0;
		representSet[u].clear();
		cent[u] = u;
	}

	dCompare =  &dLandmark;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : landmarkSet) {
		queue.push(l);
	}
	unordered_set<int> removedSet;

	while (!queue.empty()) {
		// // for (int i=0; i<5; i++) trace() << "queue " << queue.size() << " removedSet " << removedSet.size();
		int u = queue.top();
		queue.pop();
		// // for (int i=0; i<5; i++) trace() << "u " << u << " d[u] " << dLandmark[u];
		// auto tmp_q = queue; //copy the original queue to the temporary queue
		// while (!tmp_q.empty()) {
		// 	int v = tmp_q.top();
		// 	tmp_q.pop();
		// 	trace() << "v " << v << " d[v] " << dLandmark[v];
		// }
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		// if (!isLandmark[u]) representSet[cent[u]].push_back(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = dLandmark[u] + graph.getLength(u,v);
			
			if (alt - dLandmark[v] < -EPSILON){
				dLandmark[v] = alt;
				cent[v] = cent[u];
				next[v] = u;
				queue.push(v);
			}
			else if ((abs(alt - dLandmark[v]) < EPSILON) && (computeWeight(representSet[cent[u]]) < computeWeight(representSet[cent[v]]))) {
				// representSet[cent[v]].remove(v);
				cent[v] = cent[u];
				next[v] = u;
				// representSet[cent[v]].push_back(v);
			}
		}
	}
	int totalSize = 0;
	// vector<bool> check (N, false);
	// for (int l : landmarkSet) {
	// 	totalSize += representSet[l].size();
	// 	trace() << l << " " << representSet[l].size();
	// 	for (int i : representSet[l]) {
	// 		if (!check[i]) check[i] = true;
	// 		else trace() << "violate " << i;
	// 	}
	// }
	// for (int jj=0; jj<10; jj++) trace() << "total ball size " << totalSize;
	computeBallWeight();
}

double konstant2019::computeWeight(list<int> S) {
	double weight = 0;
	for (int u : A) weight += weights[u];
	return weight;
}

void konstant2019::updateCentList() {
	for (int u=0; u<numNodes; u++) {
		if ((u == self)) continue;
		double minDist = DBL_MAX;
		for (int l : A) {
			if (!graph.hasEdge(l, u) || (l == cent[u])) continue;
			double dist = graph.getLength(l, u);
			if (dist < minDist) {
				minDist = dist;
				centList.clear();
				centList[u].push_back(l);
			}
		}
		centList[u].push_back(cent[u]);
	}
}

double konstant2019::computeClusterWeight(int uNode){
	// for (int i=0; i<5; i++) trace() << "computeClusterWeight";
	vector<double> d(N, DBL_MAX);
	d[uNode] = 0;
	dCompare =  &d;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	queue.push(uNode);
	unordered_set<int> removedSet;
	while (!queue.empty()) {
		int u = queue.top();
		queue.pop();
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = d[u] + graph.getLength(u,v);
			if (alt < d[v]){
				d[v] = alt;
				if (d[v]<dLandmark[v]) queue.push(v);
			}
		}
	}
	double totalWeight = 0;
	for (int u : removedSet){
		totalWeight += weights[u];
	}
	return totalWeight;
}

vector<int> konstant2019::verifyFringeSet(){
	for (int i=0; i<1; i++) trace() << "verifyFringeSet A.size " << A.size();
	int count = 1;
	fringeSet.clear();
	innerSet.clear();
	vector<int> returnlist;
	for (int landmark : A){
		for (int i=0; i<1; i++) trace() << "landmark " << landmark;
		double weight = ballWeight.find(landmark)->second;
		if (weight > w_max[landmark]) {
			weight = 0;
			// for (int i=0; i<5; i++) trace() << "landmark " << landmark << " ballWeight " << weight << " w_max " << w_max[landmark];
			dCompare =  &dLandmark;
			priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
			for (int u : representSet[landmark]) queue.push(u);
			while (weight < w_max[landmark]){
				int u = queue.top();
				queue.pop();
				for (int i=0; i<1; i++) trace() << "inner " << u << " " << weight;
				innerSet.push_back(u);
				weight += weights[u];
				// for (int i=0; i<5; i++) trace() << "ballWeight " << weight;
			}
			while (!queue.empty()) {
				int u = queue.top();
				for (int i=0; i<1; i++) trace() << "fringe " << u;
				queue.pop();
				fringeSet.push_back(u);
			}
		}
	}
	return returnlist.empty() ? fringeSet : returnlist;
}

vector<int> konstant2019::TZ_sample2(vector<int> W, double b) {
	// for (int i=0; i<5; i++) trace() << "TZ_sample2";
	int maxTrial = 3;
	vector<int> new_W;
	while (!W.empty()){
		int i = rand() % W.size();
		int nextLandmark = W[i];
		if (isLandmark[nextLandmark]) {
			W.erase(std::remove(W.begin(), W.end(), nextLandmark), W.end());
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
			if (std::find(W.begin(), W.end(), v) != W.end()){
				W.erase(std::remove(W.begin(), W.end(), v), W.end());
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
		if ((maxTrial == 0) || (intersectBallSize>w_min)){//new_W.empty() &&
			new_W.push_back(nextLandmark);
			isLandmark[nextLandmark] = true;
			maxTrial = 3;
		}
		else{
			for (int v : intersectBall) W.push_back(v);
			if (maxTrial>0){
				maxTrial--;
				continue;
			}
			else break;
		}
	}
	//StdOut.printf("Adding... %d\n", new_W.size());
	return new_W;
}

void konstant2019::computeBallWeight(){
	// for (int i=0; i<10; i++) trace() << "computeBallWeight";
	maxBallWeight = 0;
	minBallWeight = INT64_MAX;
	// totalWeights = 0;
	for (int u : graph.getNodesExceptSink()){
		representSet[cent[u]].push_back(u);
	}

	double tempBallWeight[A.size()];
	for (int i=0; i<A.size(); i++) tempBallWeight[i] = 0;
	int size = 0;
	for (int i=0; i<A.size(); i++){
		size += representSet[A[i]].size();
		for (int u : representSet[A[i]]) {
			tempBallWeight[i] += weights[u];
			// trace() << "node " << u << " weight " << graph.getWeight(u);
		}
		// totalWeights += tempBallWeight[i];
		// if (tempBallWeight[i] > maxBallWeight) maxBallWeight = tempBallWeight[i];
		// if (tempBallWeight[i] < minBallWeight) minBallWeight = tempBallWeight[i];
	}
	ballWeight.clear();
	for (int i=0; i<A.size(); i++){
		ballWeight.insert(pair<int,double>(A[i], tempBallWeight[i]));
		// trace() << "computeBallWeight " << A[i] << " " << tempBallWeight[i];
	}
	// for (int i=0; i<10; i++) trace() << "computeBallWeight OK";
}

void konstant2019::recruitNewCHsAlpha(){
	// for (int i=0; i<10; i++) trace() << "recruitNewCHsAlpha";
	vector<int> W;//verifyFringeSet(representSet, graph.getTotalWeights()/A.size());
	if (W.empty()){
		vector<int> fringeSet = verifyFringeSet();
		// for (int jj=0; jj<10; jj++) trace() << "fringeSet " << fringeSet.size();
		for (int w : fringeSet){
			// if (w_max[w] >= w_min) 
			W.push_back(w);
		};
		// for (int jj=0; jj<10; jj++) trace() << "W " << W.size();
	}
	A2 = TZ_sample2(W, 0);
	for (int jj=0; jj<10; jj++) trace() << "A2 " << A2.size();
	if (!A2.empty()) A.insert(A.end(), A2.begin(), A2.end());
	// growBalls(A2);
	clearData();
	growBalls(A);
	// for (int i=0; i<10; i++) trace() << "recruitNewCHsAlpha OK";
}

void konstant2019::clearData(){
	for (int u : graph.getNodesExceptSink()){
		if (isLandmark[u]){
			dLandmark[u] = 0;
		}
		else {
			dLandmark[u] = DBL_MAX;
		}
		cent[u] = -1;
		next[u] = -1;
		centList[u].clear();
		representSet[u].clear();
	}
}

void konstant2019::constructClusters(){
	// for (int i=0; i<10; i++) trace() << "constructClusters";

	for (int i=0; i<N; i++) {
		if (i == sinkId) continue;
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		// trace() << "E_i " << i << " " << E_i;
		w_max[i] = Wt*(E_i-E_min)/(E_max-E_min);
		// trace() << "w_max " << i << " " << w_max[i];
	}

	int k_max;
	vector<double> w_max_ordered (w_max);
	sort(w_max_ordered.begin(), w_max_ordered.end(), greater<double>());
	double w_total_tmp = w_total;
	trace() << "w_total " << w_total;
	for (int i=0; i<N; i++) {
		trace() << "w_max_ordered " << i << " " << w_max_ordered[i];
		w_total_tmp -= w_max_ordered[i];
		if (w_total_tmp < 0) {
			k_max = i+1;
			break;
		}
		// trace() << "w_total_tmp " << w_total_tmp;
	}
	int k_min = (int)(w_total/Wt);//k_max;
	if (k_min <= 0) k_min = 1;
	k_max = k_min*1.5;
	// trace1() << "k_max " << k_max;
	w_min = w_total/k_max;
	// for (int i=0; i<10; i++) trace() << "w_min " << w_min;

	for (int l : A) {
		if (w_max[l] < w_min) {
			A.erase(std::remove(A.begin(), A.end(), l), A.end());
			isLandmark[l] = false;
		}
	}
	
	int count = 0;
	clearData();
	vector<int> satisfiedNodes;
	for (int i=0; i<N; i++) {
		if ((w_max[i] >= w_min) && !isLandmark[i]) satisfiedNodes.push_back(i);
	}
	while (A.empty()) {
		A = TZ_sample(satisfiedNodes, 8);
	}
	growBalls(A);
	bool fl;
	// for (int i=0; i<10; i++) trace() << "subloop";
	do {
		// if (A.size() > k_min) {
		// 	vector<double> tempBallWeight(N, 0);
		// 	for (auto pair : ballWeight) {
		// 		tempBallWeight[pair.first] = pair.second;
		// 	}
		// 	dCompare =  &tempBallWeight;
		// 	priority_queue<int,vector<int>, decltype(&Comparebydistance)> Q(Comparebydistance);
		// 	for (int l : A) Q.push(l);
		// 	while (A.size() > k_min){
		// 		int u = Q.top();
		// 		Q.pop();
		// 		A.erase(std::remove(A.begin(), A.end(), u), A.end());
		// 		isLandmark[u] = false;
		// 	}
		// }
		// clearData();
		// growBalls(A);
		int Asize = A.size();
		recruitNewCHsAlpha();
		// for (int i=0; i<10; i++) trace() << "recruitNewCHsAlpha " << count << " before " << Asize << " after " << A.size();
		if (count++ > 50) {
			clearData();
			growBalls(A);
			break;
		}
		// for (int i=0; i<10; i++) trace() << "Trim CH set";
		if (A.size() > k_max) {
			vector<double> tempBallWeight(N, 0);
			for (auto pair : ballWeight) {
				tempBallWeight[pair.first] = pair.second;
			}
			dCompare =  &tempBallWeight;
			priority_queue<int,vector<int>, decltype(&Comparebydistance)> Q(Comparebydistance);
			for (int l : A) Q.push(l);
			while (A.size() > k_max){
				int u = Q.top();
				Q.pop();
				A.erase(std::remove(A.begin(), A.end(), u), A.end());
				isLandmark[u] = false;
			}
		}
		clearData();
		growBalls(A);

		fl = false;
		for (auto pair : ballWeight) {
			// for (int i=0; i<5; i++) trace() << "check " << pair.first << " ballWeight " << pair.second << " w_max " << w_max[pair.first];
			if (w_max[pair.first] < pair.second) {
				fl = true;
				// for (int i=0; i<5; i++) trace() << "violate " << pair.first << " ballWeight " << pair.second << " w_max " << w_max[pair.first];
				break;
			}
		}

		// if (count++ > 50) break;
		trace() << "subloop " << count;

		nloop = count;
	} while (fl);
	if (count > 50) trace() << "violate";
	double maxWeight = 0;
	for (auto pair : ballWeight) {
		if (pair.second > maxWeight) maxWeight = pair.second;
	}
	trace() << "epsilon " << maxWeight/w_total*A.size();
	updateCentList();
	cent[self] = -1;
	for (int l : A) cent[l] = -1;
	for (int i=0; i<1; i++) trace() << "A size " << A.size();
	trace() << "Wt " << Wt;
	trace() << "k_max " << k_max;
	stringstream ss;
	for(int l : A) {
		// ss << l << " ";
		trace() << l << " " << ballWeight[l];
	}
	// trace() << ss.str();
}

void konstant2019::buildTrajectories(){
	for (int i=0; i<numUAVs; i++) trajectories[i].clear();
	if (A.size() < numUAVs) {
		for (int i=0; i<A.size(); i++) {
			trajectories[i].push_back(self);
			trajectories[i].push_back(A[i]);
			trajectories[i].push_back(self);
		}
	}
	else {
		// for (int i=0; i<10; i++) trace() << "buildTrajectories";
		// double maxLength = 0;
		// vector<int> CHvector = partitionIntoSectors();
		// for (int k=0; k<numUAVs; k++) {
		// 	vector<int> zone_k;
		// 	int start = CHvector.size()/numUAVs*k;
		// 	int end = CHvector.size()/numUAVs*(k+1);
		// 	for (int i=start; i<end; i++) {
		// 		zone_k.push_back(CHvector[i]);
		// 	}

		// 	// trajectories[k] = mainTSP(zone_k);
		// 	vector<int> trajectory_k;
		// 	trajectory_k.push_back(self);
		// 	for (int i=0; i<10; i++) trace() << "zone " << k;
		// 	stringstream ss;
		// 	for(int i : zone_k) {
		// 		ss << i << " ";
		// 	}
		// 	for (int i=0; i<10; i++) trace() << ss.str();
		// 	vector<int> CH_tr = mainTSP(zone_k);
		// 	for (int i=0; i<10; i++) trace() << "mainTSP";
		// 	trajectory_k.insert(trajectory_k.end(), CH_tr.begin(), CH_tr.end());
		// 	trajectory_k.push_back(self);
		// 	trajectories[k] = trajectory_k;
		// 	double length = calculatePathLength(trajectory_k);
		// 	for (int i=0; i<10; i++) trace() << "zone " << k << " size " << zone_k.size() << " length " << length;
		// 	if (length > maxLength) maxLength = length;
		// }

		// trajectories = callVrptw(*this, sinkId, A, numUAVs, L_max);
		trajectories = callTspga(*this, sinkId, A, numUAVs, L_max);
		while (trajectories.size() < numUAVs) {
			trajectories.push_back(vector<int>());
		}
	}

	double maxLength = 0;
	unordered_set<int> landmarks;
	landmarks.insert(A.begin(), A.end());
	for (int k=0; k<numUAVs; k++) {
		double length = calculatePathLength(trajectories[k]);
		if (length > maxLength) maxLength = length;
		trace() << "trajectory " << k << " length " << length;
		stringstream ss;
		for(int i : trajectories[k]) {
			ss << i << " ";
			landmarks.erase(i);
		}
		trace() << ss.str();
	}
	trace() << "maxLength " << maxLength;
	for (int i : landmarks) trace() << i << " is unvisited!!!";
};

vector<vector<int>> konstant2019::partitionIntoSectors() {
	for (int ii=0; ii<20; ii++) trace1() << "partitionIntoSectors";
	vector<vector<int>> Sectors;
	for (int k=0; k<numUAVs; k++) Sectors.push_back(vector<int>());
	int i0 = rand() % numNodes;
	if (i0 == sinkId) i0 = (sinkId+1) % numNodes;
	Point sinkLocation = location(sinkId);
	// trace1() << "i0 " << i0;

	auto P = GlobalLocationService::getLocation(i0);
	CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << P.x() << "\t" << P.y() << "\t" << 256000 << "\t" << i0;

	vector<double> PE (numNodes);
	double P_total = 0;
	for (int i=0; i<N; i++) {
		if (i == sinkId) continue;
		PE[i] = (E0[i] - getResMgrModule(i)->getRemainingEnergy()) / E0[i];
		// trace1() << "PE_" << i << " " << PE[i];
		P_total += PE[i];
	}
	for (int ii=0; ii<20; ii++) trace1() << "P_total" << P_total;

	for (int ii=0; ii<20; ii++) trace1() << "partitionIntoSectors 1";
	
	vector<int> sortedList;
    for (int i = 0; i < numNodes; i++) {
		if (i == sinkId) continue;
        Angle angle_i = G::angle(sinkLocation, location(i0), sinkLocation, location(i));

		// int sectorId = (int) (angle/2/M_PI*numUAVs);

		for (int ii=0; ii<20; ii++) trace1() << "angle " << sinkId << " " << i0 << " " << i << " " << angle_i;

		unsigned int j = 0;
        for (j = 0; j < sortedList.size(); j++) {
			Angle angle_j1 = G::angle(sinkLocation, location(i0), sinkLocation, location(sortedList[j]));
            if (angle_j1 > angle_i) {
                sortedList.insert(sortedList.begin() + j, i);
                break;
            }
        }

        if (sortedList.empty() || (j == sortedList.size())) {
            sortedList.push_back(i);
        }
    }

	stringstream ss;
	for(int l : sortedList) {
		ss << l << " ";
	}
	// trace1() << ss.str();
	for (int ii=0; ii<20; ii++) trace1() << "partitionIntoSectors sorted";

	int sectorId = 0;
	double sectorPercent = P_total/numUAVs;
	for (int u : sortedList) {
		Sectors[sectorId].push_back(u);
		sectorPercent -= PE[u];
		if (sectorPercent < 0) {
			sectorId++;
			sectorPercent = P_total/numUAVs;
		}
	}

	for (int ii=0; ii<20; ii++) trace1() << "partitionIntoSectors RaySectors";
	vector<vector<int>> RaySectors (numUAVs);
	for (int k=0; k<numUAVs; k++) {
		vector<int> sector = Sectors[k];
		for (int ii=0; ii<20; ii++) trace1() << "RaySectors " << k << " size " << sector.size();
		int u0 = sector[0];
		for (int ii=0; ii<20; ii++) trace1() << "u0 " << u0;
		CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << location(u0).x() << "\t" << location(u0).y() << "\t" << 512000 << "\t" << 0;
		CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << location(sector[sector.size()-1]).x() << "\t" << location(sector[sector.size()-1]).y() << "\t" << 512000 << "\t" << 0;
		Angle maxAngle = G::angle(sinkLocation, location(u0), sinkLocation, location(sector[sector.size()-1]));
		Angle ray2Angle = maxAngle/4;
		Angle ray4Angle = maxAngle*3/4;
		for (int ii=0; ii<20; ii++) trace1() << "maxAngle " << maxAngle << " ray2Angle " << ray2Angle << " ray4Angle " << ray4Angle;
		for (int u : sector) {
			Angle uAngle = G::angle(sinkLocation, location(u0), sinkLocation, location(u));
			for (int ii=0; ii<20; ii++) trace1() << "u " << u << " uAngle " << uAngle;
			if ((uAngle >= ray2Angle) && (uAngle <= ray4Angle)) {
				unsigned int j = 0;
				for (j = 0; j < RaySectors[k].size(); j++) {
					if (G::distance(location(RaySectors[k][j]), sinkLocation) > G::distance(location(u), sinkLocation)) {
						RaySectors[k].insert(RaySectors[k].begin() + j, u);
						break;
					}
				}

				if (RaySectors[k].empty() || (j == RaySectors[k].size())) {
					RaySectors[k].push_back(u);
				}
			}
		}
	}

	int sid = 0;
	for (auto sector : Sectors) {
		for (int ii=0; ii<20; ii++) trace1() << "sector " << sid << " size " << sector.size();
		stringstream ss;
		if (!sector.empty()) for(int l : sector) {
			ss << l << " ";
		}
		// trace1() << ss.str();
		sid++;
	}
	for (auto sector : RaySectors) {
		for (int ii=0; ii<20; ii++) trace1() << "raysector " << sid << " size " << sector.size();
		stringstream ss;
		if (!sector.empty()) for(int l : sector) {
			ss << l << " ";
		}
		// trace1() << ss.str();
		sid++;
	}
	for (int ii=0; ii<20; ii++) trace1() << "partitionIntoSectors done";
	return RaySectors;
}

vector<vector<int>> konstant2019::basicToursPlanning() {
	vector<vector<int>> Sectors = partitionIntoSectors();
	for (int ii=0; ii<20; ii++) trace1() << "basicToursPlanning";

	// if (!basicTours0.empty()) return basicTours0;
	
	vector<vector<int>> basicTours;
	for (int k=0; k<numUAVs; k++) {
		vector<Point> newTourPoint;
		vector<Point> innerPoint;
		innerPoint.push_back(location(sinkId));
		for (int ii=0; ii<20; ii++) trace() << "basicToursPlanning 1";
		if (Sectors[k].size() >= 1) {
			CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << location(Sectors[k][0]).x() << "\t" << location(Sectors[k][0]).y() << "\t" << 128000 << "\t" << 0;
			CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << location(Sectors[k][Sectors[k].size()-1]).x() << "\t" << location(Sectors[k][Sectors[k].size()-1]).y() << "\t" << 128000 << "\t" << 0;
			innerPoint.push_back(location(Sectors[k][0]));
			newTourPoint = innerPoint;
			double per;
			if (Sectors[k].size() >= 2) {
				int i = 1;
				for (int ii=0; ii<20; ii++) trace() << "basicToursPlanning 2";
				int n_min = 0;
				int n_max = Sectors[k].size()-1;
				int n_mid;
				do {
					for (int ii=0; ii<20; ii++) trace() << i << " " << Sectors[k][i];
					n_mid = (n_min+n_max)/2;
					innerPoint.clear();
					innerPoint.push_back(location(sinkId));
					for (int j=0; j<=n_mid; j++) {
						innerPoint.push_back(location(Sectors[k][j]));
					}
					// innerPoint.push_back(location(Sectors[k][i]));
					vector<Point> convexHull = G::convexHull(innerPoint);
					per = G::polygonPerimeter(convexHull);
					if (per <= L_max) {
						newTourPoint = convexHull;
						n_min = n_mid+1;
					} else {
						n_max = n_mid-1;
					}
					i++;
				// } while ((per <= L_max) && (i < Sectors[k].size()));
				} while (n_max >= n_min);
			}
		}
		vector<int> newTour;
		for (Point p : newTourPoint) newTour.push_back(GlobalLocationService::getId(p));
		basicTours.push_back(newTour);
	}
	for (int k=0; k<numUAVs; k++) {
		for (int ii=0; ii<20; ii++) trace() << "tour " << k << " size " << basicTours[k].size();
		for (int ii=0; ii<20; ii++) trace() << " length " << GlobalLocationService::convexHullPerimeter(basicTours[k]);
		stringstream ss;
		if (!basicTours[k].empty()) for(int l : basicTours[k]) {
			ss << l << " ";
		}
		trace() << ss.str();
	}
	for (int ii=0; ii<20; ii++) trace() << "basicToursPlanning done";
	basicTours0 = basicTours;
	return basicTours;
}

void konstant2019::findEnergyEfficientSolution() {
	vector<vector<int>> basicTours = basicToursPlanning();
	for (int k=0; k<basicTours.size(); k++) {
		for (int i=0; i<basicTours[k].size()-1; i++){
			auto P = GlobalLocationService::getLocation(basicTours[k][i]);
			auto Q = GlobalLocationService::getLocation(basicTours[k][i+1]);
			CastaliaModule::trace2() << roundNumber << "\tLINE\tred\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
		}
	}

	vector<vector<int>> finalTours = basicTours;
	for (int ii=0; ii<20; ii++) trace1() << "findEnergyEfficientSolution";

	string str0 = "";

	vector<int> pointsNotInTour;
	for (int i=0; i<numNodes; i++) pointsNotInTour.push_back(i);
	for (int k=0; k<numUAVs; k++) {
		for (int u : basicTours[k]) {
			if (find(pointsNotInTour.begin(), pointsNotInTour.end(), u) != pointsNotInTour.end())
				pointsNotInTour.erase(find(pointsNotInTour.begin(), pointsNotInTour.end(), u));
		}
	}

	for (int ii=0; ii<20; ii++) trace() << "findEnergyEfficientSolution 0";
	int retries = 2;
	int maxIter = 200;
	double cur_cnsmptn = DBL_MAX;//calculateConsumption(finalTours);
	for (int i=0; i<retries; i++) {
		for (int j=0; j<maxIter; j++) {
			for (int ii=0; ii<20; ii++) trace1() << "iter " << j << " retry " << i;
			// if (j%10 == 0) trace1() << "iter " << j;
			int rdm;
			for (int ii=0; ii<20; ii++) trace() << "findEnergyEfficientSolution 1";
			do {
				int k = rand() % basicTours.size();
				if (basicTours[k].size() > 2) {
					int h = rand() % basicTours[k].size();
					if (basicTours[k][h] != sinkId) {
						pointsNotInTour.push_back(basicTours[k][h]);
						basicTours[k].erase(basicTours[k].begin() + h);
					}
				}
				rdm = rand() % 100;
				trace() << "rdm " << rdm;
			} while (rdm < 75);

			for (int ii=0; ii<20; ii++) trace() << "findEnergyEfficientSolution 2";
			bool feasible;
			do {
				int index = rand() % pointsNotInTour.size();
				int u = pointsNotInTour[index];
				feasible = false;
				random_shuffle ( basicTours.begin(), basicTours.end() );
				for (int k=0; k<numUAVs; k++)  {
					double minPer = DBL_MAX;
					vector<int> minTour;

					for (int h=0; h<basicTours[k].size(); h++) {
						vector<int> newTour;
						vector<Point> newTourPoint;
						for (int o=0; o<=h; o++) {
							newTour.push_back(basicTours[k][o]);
							newTourPoint.push_back(location(basicTours[k][o]));
						}
						newTour.push_back(u);
						newTourPoint.push_back(location(u));
						for (int o=h+1; o<basicTours[k].size(); o++) {
							newTour.push_back(basicTours[k][o]);
							newTourPoint.push_back(location(basicTours[k][o]));
						}

						double per = G::polygonPerimeter(newTourPoint);
						if (per < minPer) {
							minPer = per;
							minTour = newTour;
						}
					}
					if (minPer <= L_max) {
						basicTours[k] = minTour;
						pointsNotInTour.erase(find(pointsNotInTour.begin(), pointsNotInTour.end(), u));
						feasible = true;
						break;
					}
					if (feasible) break;
				}
			} while (feasible);

			for (int ii=0; ii<20; ii++) trace() << "findEnergyEfficientSolution 3";
			double new_cnsmptn = calculateConsumption(basicTours);
			if (cur_cnsmptn/j > new_cnsmptn/maxIter) {
				str0 = ss0.str();
				finalTours = basicTours;
				cur_cnsmptn = new_cnsmptn;
				config.save(A, cent, next, trajectories);
			}
		}
	}
	// trace1() << str0;
	for (int ii=0; ii<20; ii++) trace() << "findEnergyEfficientSolution done";
}

double konstant2019::calculateConsumption(vector<vector<int>> tours) {
	for (int ii=0; ii<20; ii++) trace() << "calculateConsumption";
	A.clear();
	for (int i=0; i<numNodes; i++)  isLandmark[i] = false;
	clearData();
	vector<int> startIndex;
	for (int k=0; k<numUAVs; k++) {
		for (int i=0; i<tours[k].size(); i++) {
			int l = tours[k][i];
			if (l == sinkId) {
				startIndex.push_back(i);
			}
			else if (!isLandmark[l]) {
				isLandmark[l] = true;
				A.push_back(l);
			}
		}
	}
	
	
	// clusterTreeBuilding(A);
	for (int ii=0; ii<20; ii++) trace() << "growBalls";
	growBallsKonstant(A);
	for (int ii=0; ii<20; ii++) trace() << "growBalls end";
	for (int ii=0; ii<20; ii++) trace() << "A.size " << A.size();
	for (int l : A) cent[l] = -1;
	for (int i=0; i<numNodes; i++) {
		if (i == sinkId) continue;
	}
	for (int k=0; k<numUAVs; k++) trajectories[k].clear();
	for (int k=0; k<numUAVs; k++) {
		for (int i=0; i<tours[k].size(); i++) {
			int trueIndex = (i+startIndex[k])%tours[k].size();
			trajectories[k].push_back(tours[k][trueIndex]);
		}
		trajectories[k].push_back(sinkId);
	}
	NetworkConfig oldConfig;
	oldConfig.save(config);
	config.save(A, cent, next, trajectories);

	ss0.str("");

	double Er_min = DBL_MAX;
	double Er_total = 0;
	double new_cnsmptn = 0;
	for (int i=0; i<numNodes; i++) {
		double Er_i = getResMgrModule(i)->getRemainingEnergy();
		if (i == sinkId) continue;
		double rxSize = calculateRxSize(i);
		new_cnsmptn += rxEnergy(rxSize);
		Er_i -= rxEnergy(rxSize);
		ss0 << "\ne_rxEnergy " << i << " " << rxEnergy(rxSize);
		double txSize = rxSize + weights[i];
		if (config.cent[i] == -1) {
			new_cnsmptn += txEnergy(txSize, D2UAV);
			Er_i -= txEnergy(txSize, D2UAV);
			ss0 << "\ne_txEnergy " << i << " " << txEnergy(txSize, D2UAV);
		} else {
			double d2next = G::distance(selfLocation, GlobalLocationService::getLocation(config.next[self]));
			new_cnsmptn += txEnergy(txSize, d2next);
			Er_i -= txEnergy(txSize, d2next);
			ss0 << "\ne_txEnergy " << i << " " << txEnergy(txSize, d2next);
		}
		Er_total += Er_i;
		if (Er_i < Er_min) Er_min = Er_i;
	}
	config.save(oldConfig);
	double alpha = 0.9;
	// new_cnsmptn = - (alpha * Er_min + (1-alpha) * Er_total / (numNodes-1));
	return new_cnsmptn;
}


double konstant2019::clusterTreeBuilding(vector<int> A) {
	vector<double> d(numNodes, DBL_MAX);
	vector<bool> isAdded(numNodes, false);
	for (int u : A){
		d[u] = 0;
		isAdded[u] = true;
		cent[u] = u;
		next[u] = -1;
	}
	unordered_set<int> untrackedSet;
	for (int u=0; u<numNodes; u++) {
		if (isAdded[u] || u == sinkId) continue;
		untrackedSet.insert(u);
	}

	while (!untrackedSet.empty())
	{
		// trace1() << "count " << count;
		double max_val = 0;
		int v_max = -1;
		int u_max = -1;
		for (int v : untrackedSet) {
			for (int u : graph.getAdjExceptSink(v)){
				if (!isAdded[u]) continue;
				double val = getResMgrModule(u)->getRemainingEnergy() / (d[u] + distance(u,v)) / (d[u] + distance(u,v));
				if (val > max_val) {
					max_val = val;
					u_max = u;
					v_max = v;
				}
			}
		}

		if (v_max != -1) {
			d[v_max] = d[u_max] + distance(u_max,v_max);
			isAdded[v_max] = true;
			untrackedSet.erase(v_max);
			next[v_max] = u_max;
			cent[v_max] = cent[u_max];
		}
		else break;
	}
	
}