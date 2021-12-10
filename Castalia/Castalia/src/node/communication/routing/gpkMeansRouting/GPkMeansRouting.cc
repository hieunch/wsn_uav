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

#include "GPkMeansRouting.h"

Define_Module(GPkMeansRouting);

void GPkMeansRouting::startup()
{	
	/*--- The .ned file's parameters ---*/
	applicationID = par("applicationID").stringValue(); 	

	d0 = par("d0");
	gamma = par("gamma");
	countSuccess = 0;
	k_opt = -1;
	if (isSink) GPinit();
		
	setTimer(START_ROUND, 50);
}

void GPkMeansRouting::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// weights[self] = pkt->getByteLength();
		string dst(destination);
		GPkMeansRoutingPacket *netPacket = new GPkMeansRoutingPacket("GP routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setGPkMeansRoutingPacketKind(GP_ROUTING_DATA_PACKET);
		netPacket->setSource(SELF_NETWORK_ADDRESS);
		encapsulatePacket(netPacket, pkt);
		bufferAggregate.push_back(*netPacket);
	}	
}

void GPkMeansRouting::fromMacLayer(cPacket *pkt, int macAddress, double rssi, double lqi){
	GPkMeansRoutingPacket *netPacket = dynamic_cast <GPkMeansRoutingPacket*>(pkt);

	if (!netPacket)
		return;

	switch (netPacket->getGPkMeansRoutingPacketKind()) {

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

void GPkMeansRouting::timerFiredCallback(int index)
{
	switch (index) {
		
		case START_ROUND:{
			// for (int i=0; i<5; i++) trace() << "START_ROUND";
			// resMgrModule->resetBattery();
			if (isSink) {
				trace() << "START_ROUND";
				setTimer(START_CLUSTERING, 1);
				setTimer(END_ROUND, 3);
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
			mainAlg();
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
				trace1() << "E " << i << " " << E_i << " " << config.cent[i] << " " << config.next[i] << " " << rxSizes[i]/1000 << " " << energyConsumeds[i];
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
			trace1() << "E_min " << E_min;
			trace1() << "Energy\t" << E_min << "\t" << i_min << "\t" << level << "\t" << config.cent[i_min] << "\t" << calculateRxSize(i_min) << "\t" << weights[i_min] << "\t" << E_total/numNodes << "\t" << totalConsumed << "\t" << maxConsumed << "\t" << maxTxSize << "\t" << config.A.size() << "\t" << devE;
			// for (int u : fringeSet_0) debugPoint(location(u), "blue");
			// for (int u : innerSet_0) debugPoint(location(u), "yellow");
			// for (int u : A2_0) debugPoint(location(u), "green");
			break;
		}
	}
}

void GPkMeansRouting::processBufferedPacket()
{
		
}



void GPkMeansRouting::mainAlg() {

	for (int i=0; i<10; i++) trace() << "mainAlg";
	E_min = DBL_MAX;
	E_max = 0;
	for (int i=0; i<numNodes; i++) {
		if (i == self) continue;
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		if (E_i > E_max) E_max = E_i;
		if (E_i < E_min) E_min = E_i;
	}
	w_total = 0;
	for (int i=0; i<numNodes; i++) w_total += weights[i];
	k_max = numNodes;
	k_min = 1;

	if (k_opt != -1) {
		k_max = k_opt*3/2;
		k_min = k_opt*1/2;
		if (countSuccess == 10) {
			k_opt = -1;
			countSuccess = 0;
		}
	}

	for (int i=0; i<1; i++) trace() << "loop";
	bool fl;
	bool isSaved = false;
	int count = 0;
	do {
		reset();
		clearData();
		k = (k_max + k_min) / 2;
		
		for (int i=0; i<1; i++) trace1() << "k " << k << " k_max " << k_max << " k_min " << k_min;
		for (int i=0; i<1; i++) trace1() << "constructClusters";
		constructClusters2();
		for (int i=0; i<1; i++) trace1() << "buildTrajectories " << A.size();
		buildTrajectories();
		
		for (int i=0; i<10; i++) trace() << "calculatePathLength";
		fl = false;
		for (int t=0; t<numUAVs; t++) {
			if (calculatePathLength(trajectories[t]) > L_max) {
				fl = true;
				break;
			};
		}

		if (fl) {
			for (int i=0; i<10; i++) trace() << "increase Wmin";
			if (count == 5) {
				k_max = k;
				count = 0;
			}
			else {
				count++;
				continue;
			}
		} else {
			for (int i=0; i<10; i++) trace() << "save config";
			config.save(A, cent, next, trajectories);
			k_min = k;
			count = 0;
			isSaved = true;
			if (k == k_opt) {
				break;
			}
		}

		if ((k_max <= k_min+1) && !isSaved) {
			k_max = numNodes;
			k_min = 1;
			k_opt = -1;
			countSuccess = 0;
		}

	} while (k_max > k_min+1);
	if (k_opt == -1) k_opt = k;
	countSuccess++;

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

	E_min = DBL_MAX;

	for (int u=0; u<N; u++) {
		if (cent[u] >= 0) debugLine(location(u), location(cent[u]), "black");
	}
	for (int i=0; i<numUAVs; i++) {
		for (int j=0; j<trajectories[i].size()-1; j++) {
			debugLine(location(trajectories[i][j]), location(trajectories[i][j+1]), "red");
		}
	}
	trace1() << "A size " << config.A.size();
}


void GPkMeansRouting::GPinit() {
	for (int i=0; i<10; i++) trace() << "GPinit";
	
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
}

void GPkMeansRouting::reset() {
	
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

vector<int> GPkMeansRouting::TZ_sample(vector<int>W, double s) {
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

void GPkMeansRouting::growBalls(vector<int> landmarkSet){
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

double GPkMeansRouting::computeWeight(list<int> S) {
	double weight = 0;
	for (int u : A) weight += weights[u];
	return weight;
}

void GPkMeansRouting::updateCentList() {
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

double GPkMeansRouting::computeClusterWeight(int uNode){
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

vector<int> GPkMeansRouting::verifyFringeSet(){
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

vector<int> GPkMeansRouting::TZ_sample2(vector<int> W, double b) {
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

void GPkMeansRouting::computeBallWeight(){
	// for (int i=0; i<10; i++) trace() << "computeBallWeight";
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
	// for (int i=0; i<10; i++) trace() << "computeBallWeight OK";
}

void GPkMeansRouting::recruitNewCHsAlpha(){
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

void GPkMeansRouting::clearData(){
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

void GPkMeansRouting::constructClusters(){
	
}

void GPkMeansRouting::constructClusters2(){
	vector<int> nodes;
	for (int i=0; i<numNodes; i++) if (i != sinkId) nodes.push_back(i);
	A.clear();
	auto Clusters = kMeansClustering(nodes, k);
	for (int i=0; i<10; i++) trace() << "kMeansClustering";
	for (auto p : Clusters) {
		auto list_member = p.second;
		double E_max = 0;
		int u_max = p.first;
		for (auto u : list_member) {
			if (u == sinkId) continue;
			double E_u = getResMgrModule(u)->getRemainingEnergy();
			if (E_u > E_max) {
				E_max = E_u;
				u_max = u;
			}
		}

		A.push_back(u_max);
		cent[u_max] = -1;
		for (auto u : list_member) {
			if (u != u_max) cent[u] = u_max;
		}
		updateNextNode(u_max, list_member);
		for (int i=0; i<10; i++) trace() << "updateNextNode";
	}

	// vector<int> RPfull;
	// RPfull.push_back(self);
	// RPfull.insert(RPfull.end(), A.begin(), A.end());
	// vector<int> trajectory = mainTSP(RPfull);
}

void GPkMeansRouting::updateNextNode(int cluster_head, vector<int> list_member){
	vector<bool> isMember(N, false);
	trace1() << "CH " << cluster_head;
	for (auto u : list_member) {
		isMember[u] = true;
	}
	vector<int> nodetrace(N, -1);
	vector<double> d(N, DBL_MAX);
	d[cluster_head] = 0;
	dCompare =  &d;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	queue.push(cluster_head);
	unordered_set<int> removedSet;
	while (!queue.empty()) {
		int u = queue.top();
		queue.pop();
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		next[u] = nodetrace[u];
		for (int i=0; i<10; i++) trace() << u;
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			if (!isMember[v]) continue;
			double alt = d[u] + graph.getLength(u,v);
			if (alt < d[v]) {
				d[v] = alt;
				nodetrace[v] = u;
				queue.push(v);
			}
		}
	}
	next[cluster_head] = -1;

	for (auto u : list_member) {
		trace1() << "next[" << u << "] = " << next[u];
	}
	for (auto u : list_member) {
		trace1() << "cent[" << u << "] = " << cent[u];
	}
}

void GPkMeansRouting::buildTrajectories(){
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
		// vector<int> CHvector = sortedCHVector();
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

		trajectories = Vrptw::call(*this, sinkId, A, numUAVs, L_max);
		// trajectories = callTspga(*this, sinkId, A, numUAVs, L_max);
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

vector<int> GPkMeansRouting::sortedCHVector() {
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