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

#include "wu2019.h"

Define_Module(wu2019);

void wu2019::startup()
{	
	/*--- The .ned file's parameters ---*/
	applicationID = par("applicationID").stringValue(); 	

	d0 = par("d0");
	gamma = par("gamma");
	alpha_opt = 0;
	if (isSink) init();
		
	setTimer(START_ROUND, 50);
}

void wu2019::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// weights[self] = pkt->getByteLength();
		string dst(destination);
		wu2019Packet *netPacket = new wu2019Packet("GP routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setWu2019PacketKind(GP_ROUTING_DATA_PACKET);
		netPacket->setSource(SELF_NETWORK_ADDRESS);
		encapsulatePacket(netPacket, pkt);
		bufferAggregate.push_back(*netPacket);
	}	
}

void wu2019::fromMacLayer(cPacket *pkt, int macAddress, double rssi, double lqi){
	wu2019Packet *netPacket = dynamic_cast <wu2019Packet*>(pkt);

	if (!netPacket)
		return;

	switch (netPacket->getWu2019PacketKind()) {

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

void wu2019::timerFiredCallback(int index)
{
	switch (index) {
		
		case START_ROUND:{
			// for (int i=0; i<5; i++) trace() << "START_ROUND";
			
			if (isSink) {
				trace() << "START_ROUND";
				setTimer(START_MAINALG, 1);
				setTimer(END_ROUND, 3);
			} else {
				setTimer(SEND_DATA, 2);
			}
			roundNumber++;
			setTimer(START_ROUND, roundLength);
			// for (int i=0; i<5; i++) trace() << "setTimer " << (roundLength + simTime());
			break;
		}
		case START_MAINALG:{	
			// for (int i=0; i<5; i++) trace() << "START_MAINALG";
			totalConsumed = 0;
			maxConsumed = 0;
			mainAlg();
			break;
		}
		
		case SEND_DATA:{
			// for (int i=0; i<5; i++) trace() << "SEND_DATA";
			sendData();
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
				// trace1() << "E " << i << " " << E_i;
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
			// trace1() << "E_min " << E_min;
			trace1() << "Energy\t" << E_min << "\t" << E_total/numNodes << "\t" << totalConsumed << "\t" << maxConsumed << "\t" << maxTxSize << "\t" << config.A.size() << "\t" << devE;
			break;
		}
	}
}

void wu2019::processBufferedPacket()
{
		
}





void wu2019::mainAlg() {
	
	bool fl;
	int alpha = 0;
	bool isSaved = false;
	do {
		reset();
		clearData();
		alpha++;
		// if (alpha_opt != 0) alpha = alpha_opt;
		for (int i=0; i<1; i++) trace() << "alphaHopClustering " << alpha;
		alphaHopClustering(alpha);
		for (int i=0; i<10; i++) trace() << "buildTrajectories " << A.size();
		buildTrajectories();
		
		for (int i=0; i<10; i++) trace() << "calculatePathLength";
		fl = false;
		for (int t=0; t<numUAVs; t++) {
			// for (int i=0; i<10; i++) trace() << t;
			if (calculatePathLength(trajectories[t]) > L_max) {
				trace() << "calculatePathLength " << t << " " << calculatePathLength(trajectories[t]);
				fl = true;
				break;
			};
		}

		if (!fl) {
			for (int i=0; i<1; i++) trace() << "save config";
			config.save(A, cent, next, trajectories);
			isSaved = true;
		}
	} while (fl);

	if (alpha_opt == 0) alpha_opt = alpha;

	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;
	}
	trace1() << "maxLength " << maxLength;
	trace1() << "alpha " << alpha;
	trace1() << "A size " << A.size();
}


void wu2019::init() {
	// for (int i=0; i<10; i++) trace() << "init";
	
	N = numNodes;
	graph.init(numNodes, self, d0);
	trajectories.resize(numUAVs, vector<int>());

	d2CH.resize(N, 0.);
	cent.resize(N, -1);
	next.resize(N, -1);
	centList.resize(N);
	isCH.resize(N, false);
	representSet.resize(N);
	w_max.resize(N);
}

void wu2019::reset() {
	
	trajectories = vector<vector<int>>(numUAVs, vector<int>());

	A.clear();
	d2CH = vector<double>(N, 0.);
	cent = vector<int>(N, -1);
	next = vector<int>(N, -1);
	centList = vector<list<int>>(N);
	isCH = vector<bool>(N, false);
	representSet = vector<list<int>>(N);
	w_max = vector<double>(N, 0);
}

void wu2019::growBalls(vector<int> landmarkSet){
	// for (int i=0; i<20; i++) trace() << "growBalls Asize " << A.size();
	for (int u : landmarkSet) {
		d2CH[u] = 0;
		representSet[u].clear();
		cent[u] = u;
	}

	dCompare =  &d2CH;
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
		if (!isCH[u]) representSet[cent[u]].push_back(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = d2CH[u] + graph.getLength(u,v);
			
			if (alt - d2CH[v] < -EPSILON){
				d2CH[v] = alt;
				cent[v] = cent[u];
				next[v] = u;
				queue.push(v);
			}
		}
	}
}

void wu2019::clearData(){
	for (int u : graph.getNodesExceptSink()){
		if (isCH[u]){
			d2CH[u] = 0;
		}
		else {
			d2CH[u] = DBL_MAX;
		}
		cent[u] = -1;
		next[u] = -1;
		centList[u].clear();
		representSet[u].clear();
	}
}

void wu2019::alphaHopClustering(int alpha){
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

void wu2019::buildTrajectories(){
	for (int jj=0; jj<10; jj++) trace() << "buildTrajectories " << A.size();
	for (int i=0; i<numUAVs; i++) trajectories[i].clear();
	if (A.size() < numUAVs) {
		for (int i=0; i<A.size(); i++) {
			trajectories[i].push_back(self);
			trajectories[i].push_back(A[i]);
			trajectories[i].push_back(self);
		}
	}
	else {
		// bool success;
		// double L0 = L_max;
		// vector<vector<int>> trajectories_0;
		// do {
		// 	for (int i=0; i<numUAVs; i++) trajectories[i].clear();
		// 	unordered_set<int> Unvisited;
		// 	Unvisited.insert(A.begin(), A.end());
		// 	int current = self;
		// 	int i = 0;
		// 	while (!Unvisited.empty() && (i < numUAVs)) {
		// 		trace() << "AAAAA " << i;
		// 		double L = 0;
		// 		trajectories[i].push_back(current);
		// 		bool isFinished = true;
		// 		while (isFinished) {
		// 			double dmin = DBL_MAX;
		// 			int candidate = -1;
		// 			for (int next : Unvisited) {
		// 				double d = G::distance(location(current), location(next));
		// 				if (d < dmin) {
		// 					dmin = d;
		// 					candidate = next;
		// 				}
		// 			}

		// 			if (L+dmin+G::distance(location(candidate), selfLocation) <= L0) {
		// 				L += dmin;
		// 				trajectories[i].push_back(candidate);
		// 				current = candidate;
		// 				Unvisited.erase(current);
		// 			} else {
		// 				i++;
		// 				current = self;
		// 				isFinished = false;
		// 			}
		// 		}
		// 	}

		// 	if (!Unvisited.empty()) {
		// 		trajectories[0].insert(trajectories[0].end(), Unvisited.begin(), Unvisited.end());
		// 		success = false;
		// 	} else {
		// 		trajectories_0 = trajectories;
		// 		L0 -= 10;
		// 		success = true;
		// 	}
		// 	for (int jj=0; jj<10; jj++) trace() << "buildTrajectories " << success;
		// } while (success);

		// if (!trajectories_0.empty()) trajectories = trajectories_0;
		// for (int k=0; k<trajectories.size(); k++) {
		// 	trace() << "push_back self";
		// 	if (trajectories[k].size() > 1) trajectories[k].push_back(self);
		// }
		// trace() << "finish";

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
}