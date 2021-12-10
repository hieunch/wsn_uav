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

#include "GPRouting.h"

Define_Module(GPRouting);

vector<double> GPRouting::weights;

void GPRouting::startup()
{	
	/*--- The .ned file's parameters ---*/
	applicationID = par("applicationID").stringValue(); 	

	d0 = par("d0");
	if (isSink) GPinit();
	
	if (isSink) {
		sinkId = self;
		weights.resize(numNodes, 2000);
		L_max = par("maxUAVLength");
		gamma = par("gamma");
	}
	setTimer(START_ROUND, 50);
}

void GPRouting::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// weights[self] = pkt->getByteLength();
		string dst(destination);
		GPRoutingPacket *netPacket = new GPRoutingPacket("GP routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setGPRoutingPacketKind(GP_ROUTING_DATA_PACKET);
		netPacket->setSource(SELF_NETWORK_ADDRESS);
		encapsulatePacket(netPacket, pkt);
		bufferAggregate.push_back(*netPacket);
	}	
}

void GPRouting::fromMacLayer(cPacket *pkt, int macAddress, double rssi, double lqi){
	GPRoutingPacket *netPacket = dynamic_cast <GPRoutingPacket*>(pkt);

	if (!netPacket)
		return;

	switch (netPacket->getGPRoutingPacketKind()) {

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

void GPRouting::timerFiredCallback(int index)
{
	switch (index) {
		
		case START_ROUND:{
			for (int i=0; i<5; i++) trace() << "START_ROUND";
			
			if (isSink) {
				setTimer(START_CLUSTERING, 1);
			} else {
				
			}
			roundNumber++;
			// setTimer(START_ROUND, roundLength);
			// for (int i=0; i<5; i++) trace() << "setTimer " << (roundLength + simTime());
			break;
		}
		case START_CLUSTERING:{	
			mainAlg();
			break;
		}
		
		case START_SLOT:{
			sendAggregate();
			trace() << "Send aggregated packet to " << -1;
			processBufferedPacket();
			break;
		}
	}
}

void GPRouting::sendAggregate()
{
	
}

void GPRouting::processBufferedPacket()
{
		
}

bool Comparebydistance(int lhs, int rhs) {
	return (dCompare->at(lhs) > dCompare->at(rhs));
}





void GPRouting::mainAlg() {
	for (int i=0; i<10; i++) trace() << "mainAlg";
	double E_min = DBL_MAX;
	double E_max = 0;
	for (int i=0; i<numNodes; i++) {
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		if (E_i > E_max) E_max = E_i;
		if (E_i < E_min) E_min = E_i;
	}
	totalWeights = 0;
	for (int i=0; i<numNodes; i++) totalWeights += weights[i];
	E_min -= (txEnergy(totalWeights/numUAVs, D2UAV)+rxEnergy(totalWeights/numUAVs));
	// E_opt = E_opt*gamma;

	for (int i=0; i<10; i++) trace() << "loop";
	bool fl;
	do {
		reset();
		clearData();
		E_opt = (E_max+E_min)/2;
		for (int i=0; i<10; i++) trace() << "E_opt " << E_opt;
		for (int i=0; i<10; i++) trace() << "constructClusters";
		constructClusters();
		for (int i=0; i<10; i++) trace() << "buildTrajectories";
		buildTrajectories();
		
		for (int i=0; i<10; i++) trace() << "calculatePathLength";
		fl = false;
		for (int t=0; t<numUAVs; t++) {
			for (int i=0; i<10; i++) trace() << t;
			if (calculatePathLength(trajectories[t]) > L_max) {
				fl = true;
				break;
			};
		}

		if (fl) {
			for (int i=0; i<10; i++) trace() << "Emax half";
			E_max = E_opt;
		} else {
			for (int i=0; i<10; i++) trace() << "save config";
			config.save(A, cent, trajectories);
			E_min = E_opt;
		}
	} while (E_max-E_min>0.1);
}


void GPRouting::GPinit() {
	for (int i=0; i<10; i++) trace() << "GPinit";
	
	N = numNodes;
	graph.init(numNodes, self, d0);
	trajectories.resize(numUAVs);

	dLandmark.resize(N, 0.);
	cent.resize(N, -1);
	centList.resize(N);
	isLandmark.resize(N, false);
	representSet.resize(N);
	w_max.resize(N);
}

void GPRouting::reset() {
	
	trajectories = vector<vector<int>>(numUAVs);

	A.clear();
	dLandmark = vector<double>(N, 0.);
	cent = vector<int>(N, -1);
	centList = vector<list<int>>(N);
	isLandmark = vector<bool>(N, false);
	representSet = vector<list<int>>(N);
	w_max = vector<double>(N, 0);
}

vector<int> GPRouting::TZ_sample(vector<int>W, double s) {
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

void GPRouting::growBalls(vector<int> landmarkSet){
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
		if (!isLandmark[u]) representSet[cent[u]].push_back(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = dLandmark[u] + graph.getLength(u,v);
			
			if (alt - dLandmark[v] < -EPSILON){
				dLandmark[v] = alt;
				cent[v] = cent[u];
				queue.push(v);
			}
			// } else if ((abs(alt - dLandmark[v]) < EPSILON) && (representSet[cent[u]].size() < representSet[cent[v]].size()-1)) {
			// 	representSet[cent[v]].remove(v);
			// 	cent[v] = cent[u];
			// 	representSet[cent[v]].push_back(v);
			// }
		}
	}
	int totalSize = 0;
	vector<bool> check (N, false);
	for (int l : landmarkSet) {
		totalSize += representSet[l].size();
		trace() << l << " " << representSet[l].size();
		for (int i : representSet[l]) {
			if (!check[i]) check[i] = true;
			else trace() << "violate " << i;
		}
	}
	for (int jj=0; jj<10; jj++) trace() << "total ball size " << totalSize;
	computeBallWeight();
}

void GPRouting::updateCentList() {
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

double GPRouting::computeClusterWeight(int uNode){
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

vector<int> GPRouting::verifyFringeSet(){
	// for (int i=0; i<5; i++) trace() << "verifyFringeSet A.size " << A.size();
	int count = 1;
	vector<int> fringeSet;
	vector<int> returnlist;
	for (int landmark : A){
		// for (int i=0; i<5; i++) trace() << "landmark " << landmark;
		double weight = ballWeight.find(landmark)->second;
		// for (int i=0; i<5; i++) trace() << "ballWeight " << weight;
		if (weight > w_max[landmark]) {
			dCompare =  &dLandmark;
			priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
			for (int u : representSet[landmark]) queue.push(u);
			while (weight > w_max[landmark]){
				int u = queue.top();
				queue.pop();
				// for (int i=0; i<5; i++) trace() << "queue.size " << queue.size() << " u " << u;
				fringeSet.push_back(u);
				weight -= weights[u];
				// for (int i=0; i<5; i++) trace() << "ballWeight " << weight;
			}
		}
	}
	return returnlist.empty() ? fringeSet : returnlist;
}

vector<int> GPRouting::TZ_sample2(vector<int> W, double b) {
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
		if (((maxTrial == 0)) || (intersectBallSize>maxThres/2)){//new_W.empty() &&
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

void GPRouting::computeBallWeight(){
	for (int i=0; i<10; i++) trace() << "computeBallWeight";
	maxBallWeight = 0;
	minBallWeight = INT64_MAX;
	// totalWeights = 0;
	for (int u : graph.getNodesExceptSink()){
		representSet[cent[u]].push_back(u);
	}

	double tempBallWeight[A.size()] = {0};
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
	for (int i=0; i<10; i++) trace() << "computeBallWeight OK";
}

void GPRouting::recruitNewCHsAlpha(){
	for (int i=0; i<10; i++) trace() << "recruitNewCHsAlpha";
	vector<int> W;//verifyFringeSet(representSet, graph.getTotalWeights()/A.size());
	if (W.empty()){
		vector<int> fringeSet = verifyFringeSet();
		for (int jj=0; jj<10; jj++) trace() << "fringeSet " << fringeSet.size();
		for (int w : fringeSet){
			if (w_max[w] >= w_min) W.push_back(w);
		};
		for (int jj=0; jj<10; jj++) trace() << "W " << W.size();
	}
	vector<int> A2 = TZ_sample2(W, 0);
	for (int jj=0; jj<10; jj++) trace() << "A2 " << A2.size();
	if (!A2.empty()) A.insert(A.end(), A2.begin(), A2.end());
	// growBalls(A2);
	clearData();
	growBalls(A);
	// for (int i=0; i<10; i++) trace() << "recruitNewCHsAlpha OK";
}

void GPRouting::clearData(){
	for (int u : graph.getNodesExceptSink()){
		if (isLandmark[u]){
			dLandmark[u] = 0;
		}
		else {
			dLandmark[u] = DBL_MAX;
		}
		cent[u] = -1;
		centList[u].clear();
		representSet[u].clear();
	}
}

void GPRouting::constructClusters(){
	// for (int i=0; i<10; i++) trace() << "constructClusters";
	w_total = 0;
	for (int i=0; i<N; i++) {
		w_total += weights[i];
	}

	for (int i=0; i<N; i++) {
		double Ec_i = getResMgrModule(i)->getRemainingEnergy() - E_opt;
		trace() << "Ec_i " << i << " " << Ec_i;
		w_max[i] = estimateMaxWeight(Ec_i);
		trace() << "w_max " << i << " " << w_max[i];
	}

	int k_max;
	vector<double> w_max_ordered (w_max);
	sort(w_max_ordered.begin(), w_max_ordered.end(), greater<double>());
	double w_total_tmp = w_total;
	for (int i=0; i<N; i++) {
		w_total_tmp -= w_max[i];
		if (w_total_tmp < 0) {
			k_max = i+1;
			break;
		}
	}
	k_max = k_max*4;
	for (int i=0; i<10; i++) trace() << "k_max " << k_max;
	w_min = w_total/k_max;
	for (int i=0; i<10; i++) trace() << "w_min " << w_min;

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
		if ((w_max[i] > w_min) && !isLandmark[i]) satisfiedNodes.push_back(i);
	}
	while (A.empty()) {
		A = TZ_sample(satisfiedNodes, 8);
	}
	growBalls(A);
	bool fl;
	for (int i=0; i<10; i++) trace() << "subloop";
	do {
		int Asize = A.size();
		recruitNewCHsAlpha();
		for (int i=0; i<10; i++) trace() << "recruitNewCHsAlpha " << count << " before " << Asize << " after " << A.size();
		if (count++ > 50) {
			clearData();
			growBalls(A);
			break;
		}
		for (int i=0; i<10; i++) trace() << "Trim CH set";
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
			if (w_max[pair.first] < pair.second) {
				fl = true;
				break;
			}
		}

		nloop = count;
	} while (fl);
	updateCentList();
	cent[self] = -1;
	for (int i=0; i<10; i++) trace() << "A size " << A.size();
}

double GPRouting::calculatePathLength(vector<int> path) {
	if (path.empty()) return 0;
	double pathLength = 0;
	for (int i=0; i<path.size()-1; i++){
		pathLength += G::distance(GlobalLocationService::getLocation(path[i]), 
			GlobalLocationService::getLocation(path[i+1]));
	}
	return pathLength;
}

void GPRouting::buildTrajectories(){
	for (int i=0; i<numUAVs; i++) trajectories[i].clear();
	if (A.size() < numUAVs) {
		for (int i=0; i<A.size(); i++) {
			trajectories[i].push_back(self);
			trajectories[i].push_back(A[i]);
			trajectories[i].push_back(self);
		}
		return;
	}

	for (int i=0; i<10; i++) trace() << "buildTrajectories";
	double maxLength = 0;
	vector<int> CHvector = sortedCHVector();
	for (int k=0; k<numUAVs; k++) {
		vector<int> zone_k;
		int start = CHvector.size()/numUAVs*k;
		int end = CHvector.size()/numUAVs*(k+1);
		for (int i=start; i<end; i++) {
			zone_k.push_back(CHvector[i]);
		}

		// trajectories[k] = mainTSP(zone_k);
		vector<int> trajectory_k;
		trajectory_k.push_back(self);
		vector<int> CH_tr = mainTSP(zone_k);
		trajectory_k.insert(trajectory_k.end(), CH_tr.begin(), CH_tr.end());
		trajectory_k.push_back(self);
		trajectories[k] = trajectory_k;
		double length = calculatePathLength(trajectory_k);
		for (int i=0; i<10; i++) trace() << "zone " << k << " size " << zone_k.size() << " length " << length;
		for (int u : trajectory_k) trace() << u;
		if (length > maxLength) maxLength = length;
	}
	for (int i=0; i<10; i++) trace() << "maxLength " << maxLength;

	// trajectories = callVrptw(*this, sinkId, A, numUAVs, L_max);
	// while (trajectories.size() < numUAVs) {
	// 	trajectories.push_back(vector<int>());
	// }
};

vector<int> GPRouting::sortedCHVector() {
	vector<int> CHvector;
    if (A.empty()) {
        return CHvector;
    }
    CHvector.push_back(A[0]);

    unsigned int i = 1;
    for (i = 1; i < A.size(); i++) {
        Angle angle = G::angle(selfLocation, location(CHvector[0]), selfLocation, location(A[i]));

        unsigned int j = 0;
        for (j = 0; j + 1 < CHvector.size(); j++) {
            if (G::angle(selfLocation, location(CHvector[0]), selfLocation, location(CHvector[j + 1])) > angle) {
                CHvector.insert(CHvector.begin() + j + 1, A[i]);
                break;
            }
        }

        if (j == CHvector.size() - 1) {
            CHvector.push_back(A[i]);
        }
    }
	return CHvector;
}