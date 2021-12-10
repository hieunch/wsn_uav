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

#include "globecom.h"

Define_Module(globecom);

void globecom::startup()
{	
	/*--- The .ned file's parameters ---*/
	applicationID = par("applicationID").stringValue(); 	

	d0 = par("d0");
	gamma = par("gamma");
	k_opt = -1;
	if (isSink) init();
		
	setTimer(START_ROUND, 50);
}

void globecom::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// weights[self] = pkt->getByteLength();
		string dst(destination);
		globecomPacket *netPacket = new globecomPacket("GP routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setGlobecomPacketKind(GP_ROUTING_DATA_PACKET);
		netPacket->setSource(SELF_NETWORK_ADDRESS);
		encapsulatePacket(netPacket, pkt);
		bufferAggregate.push_back(*netPacket);
	}	
}

void globecom::fromMacLayer(cPacket *pkt, int macAddress, double rssi, double lqi){
	globecomPacket *netPacket = dynamic_cast <globecomPacket*>(pkt);

	if (!netPacket)
		return;

	switch (netPacket->getGlobecomPacketKind()) {

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

void globecom::timerFiredCallback(int index)
{
	switch (index) {
		
		case START_ROUND:{
			// for (int i=0; i<5; i++) trace() << "START_ROUND";
			
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
			// for (int i=0; i<5; i++) trace() << "START_CLUSTERING";
			totalConsumed = 0;
			maxConsumed = 0;
			mainAlg();
			break;
		}
		
		case START_SLOT:{
			// for (int i=0; i<5; i++) trace() << "START_SLOT";
			sendAggregate();
			// trace() << "Send aggregated packet to " << -1;
			// processBufferedPacket();
			break;
		}
		case END_ROUND:{	
			E_min = DBL_MAX;
			double E_total = 0;
			for (int i=0; i<numNodes; i++) {
				if (i == self) continue;
				double E_i = getResMgrModule(i)->getRemainingEnergy();
				if (E_i < E_min) E_min = E_i;
				E_total += E_i;
				trace1() << "E " << i << " " << E_i;
			}
			// trace1() << "E_min " << E_min;
			trace1() << "Energy\t" << E_min << "\t" << E_total/numNodes << "\t" << totalConsumed << "\t" << maxConsumed;
			break;
		}
	}
}

void globecom::processBufferedPacket()
{
		
}





void globecom::mainAlg() {

	vector<int> nodes;
	for (int i=0; i<numNodes; i++) nodes.push_back(i);
	
	int k = 0;
	bool fl;
	do {
		k++;
		trace() << "k = " << k;
		fl = false;
		A.clear();
		auto Clusters = kMeansClustering(nodes, k);
		for (auto p : Clusters) {
			int l = p.first;
			A.push_back(l);
			for (int u : p.second) {
				if (G::distance(location(u), location(l)) > 100) {
					fl = true;
					break;
				}
			}
			if (fl) break;
		}
	} while (fl);

	for (int jj=0; jj<10; jj++) trace() << "buildTrajectories";
	buildTrajectories();
	clearData();
	for (int l : A) isLandmark[l] = true;
	growBalls(A);
	config.save(A, cent, next, trajectories);

	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;
	}
	trace1() << "maxLength " << maxLength;
	trace1() << "A size " << config.A.size();
}


void globecom::init() {
	// for (int i=0; i<10; i++) trace() << "init";
	
	N = numNodes;
	graph.init(numNodes, self, d0);
	trajectories.resize(numUAVs, vector<int>());

	dLandmark.resize(N, 0.);
	cent.resize(N, -1);
	next.resize(N, -1);
	centList.resize(N);
	isLandmark.resize(N, false);
	representSet.resize(N);
	w_max.resize(N);
}

void globecom::reset() {
	
	trajectories = vector<vector<int>>(numUAVs, vector<int>());

	A.clear();
	dLandmark = vector<double>(N, 0.);
	cent = vector<int>(N, -1);
	next = vector<int>(N, -1);
	centList = vector<list<int>>(N);
	isLandmark = vector<bool>(N, false);
	representSet = vector<list<int>>(N);
	w_max = vector<double>(N, 0);
}

void globecom::growBalls(vector<int> landmarkSet){
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
		int u = queue.top();
		queue.pop();
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		if (!isLandmark[u]) representSet[cent[u]].push_back(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = dLandmark[u] + graph.getLength(u,v);
			
			if (alt - dLandmark[v] < -EPSILON){
				dLandmark[v] = alt;
				cent[v] = cent[u];
				next[v] = u;
				queue.push(v);
			}
		}
	}
}

void globecom::clearData(){
	for (int u : graph.getNodesExceptSink()){
		isLandmark[u] = false;
		dLandmark[u] = DBL_MAX;
		cent[u] = -1;
		next[u] = -1;
		centList[u].clear();
		representSet[u].clear();
	}
}

void globecom::alphaHopClustering(int alpha){
	vector<double> Weight (N);
	vector<double> Davg (N, 0);
	vector<int> M (N, 0);
	double Dmax = 0;
	double Mmax = 0;
	for (int i=0; i<N; i++) {
		if (i == self) continue;
		M[i] = graph.getAdjExceptSink(i).size();
		for (int j : graph.getAdjExceptSink(i)) {
			Davg[i] += graph.getLength(i, j);
		}
		Davg[i] = Davg[i]/M[i];
		if (M[i] > Mmax) Mmax = M[i];
		if (Davg[i] > Dmax) Dmax = Davg[i];
	}
	for (int i=0; i<N; i++) {
		if (i == self) continue;
		Weight[i] = - Dmax/Davg[i] * M[i]/Mmax;
	}

	dCompare =  &Weight;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> p_queue(Comparebydistance);
	for (int i=0; i<N; i++) {
		if (i == self) continue;
		p_queue.push(i);
	}
	vector<bool> isRemoved (N, false);
	while (!p_queue.empty()) {
		int i = p_queue.top();
		p_queue.pop();
		if (isRemoved[i]) continue;
		isRemoved[i] = true;
		A.push_back(i);

		vector<int> dhop (N, INT32_MAX);
		dhop[i] = 0;
		vector<bool> isVisited (N, false);
  		isVisited[i] = true;
		queue<int> q;
		q.push(i);
		while (!q.empty()) {
			int u = q.front(); q.pop();
			for (int v : graph.getAdjExceptSink(u)) if (!isVisited[v]) {
				isVisited[v] = true;
				dhop[v] = dhop[u] + 1;
				if (dhop[v] <= alpha) {
					isRemoved[v] = true;
					cent[v] = i;
					next[v] = u;
					if (dhop[v] < alpha) q.push(v);
				}
			}
		}
	}
}

void globecom::buildTrajectories(){
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

		// trajectories = callVrptw(*this, sinkId, A, numUAVs, L_max);
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
}