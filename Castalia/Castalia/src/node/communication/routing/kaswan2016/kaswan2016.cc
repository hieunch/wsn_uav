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

#include "kaswan2016.h"

Define_Module(kaswan2016);

void kaswan2016::startup()
{	
	/*--- The .ned file's parameters ---*/
	applicationID = par("applicationID").stringValue(); 	

	d0 = par("d0");
	gamma = par("gamma");
	k_opt = -1;
	if (isSink) init();
		
	setTimer(START_ROUND, 50);
}

void kaswan2016::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// weights[self] = pkt->getByteLength();
		string dst(destination);
		kaswan2016Packet *netPacket = new kaswan2016Packet("GP routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setKaswan2016PacketKind(GP_ROUTING_DATA_PACKET);
		netPacket->setSource(SELF_NETWORK_ADDRESS);
		encapsulatePacket(netPacket, pkt);
		bufferAggregate.push_back(*netPacket);
	}	
}

void kaswan2016::fromMacLayer(cPacket *pkt, int macAddress, double rssi, double lqi){
	kaswan2016Packet *netPacket = dynamic_cast <kaswan2016Packet*>(pkt);

	if (!netPacket)
		return;

	switch (netPacket->getKaswan2016PacketKind()) {

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

void kaswan2016::timerFiredCallback(int index)
{
	switch (index) {
		
		case START_ROUND:{
			// for (int i=0; i<5; i++) trace() << "START_ROUND";
			
			if (isSink) {
				trace() << "START_ROUND";
				setTimer(START_CLUSTERING, 1);
				setTimer(END_ROUND, 3);
				trace1() << "Energy\tE_min\ti_min\tlevel\tconfig.cent[i_min]\tcalculateRxSize(i_min)\tweights[i_min]\tE_total/numNodes\ttotalConsumed\tmaxConsumed\tmaxTxSize\tconfig.A.size()\tdevE\tavgConsumed\tobjective value\tE0_min\ttime_elapse\ttotalCollected\tmaxLengthRatio";
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
			// for (int i=0; i<5; i++) trace() << "START_SLOT";
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
				// trace1() << "E " << i << " " << E_i << " " << config.cent[i] << " " << config.next[i] << " " << rxSizes[i]/1000 << " " << energyConsumeds[i];
			}
			double E0_min = E_min;
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
			double objVal = 0;
			// trace1() << "E_min " << E_min;
			trace1() << "Energy\t" << E_min << "\t" << i_min << "\t" << level << "\t" << config.cent[i_min] << "\t" << calculateRxSize(i_min) << "\t" << weights[i_min] << "\t" << E_total/numNodes << "\t" << totalConsumed << "\t" << maxConsumed << "\t" << maxTxSize << "\t" << config.A.size() << "\t" << devE << "\t" << avgConsumed << "\t" << objVal  << "\t" << E0_min << "\t" << time_elapse.count() << "\t" << totalCollected << "\t" << maxLengthRatio;
			break;
		}
	}
}

void kaswan2016::processBufferedPacket()
{
		
}





void kaswan2016::mainAlg() {
	for (int i=0; i<10; i++) trace1() << "mainAlg";
	trajectories = vector<vector<int>>(1);
	vector<Point> EP (8);
	double x_max, y_max, x_total, y_total;
	x_max = y_max = x_total = y_total = 0;
	double x_min, y_min;
	x_min = y_min = DBL_MAX;
	for (int i=0; i<numNodes; i++) {
		Point p_i = location(i);
		x_total += p_i.x_;
		y_total += p_i.y_;
		if (p_i.x_ > x_max) x_max = p_i.x_;
		if (p_i.x_ < x_min) x_min = p_i.x_;
		if (p_i.y_ > y_max) y_max = p_i.y_;
		if (p_i.y_ < y_min) y_min = p_i.y_;
	}
	EP[0] = Point(x_min, y_min);
	EP[1] = Point(x_max/2, y_min);
	EP[2] = Point(x_max, y_min);
	EP[3] = Point(x_max, y_max/2);
	EP[4] = Point(x_max, y_max);
	EP[5] = Point(x_max/2, y_max);
	EP[6] = Point(x_min, y_max);
	EP[7] = Point(x_min, y_max/2);
	Point CTR = Point(x_total/numNodes, y_total/numNodes);
	double MDD = 0;
	for (int i=0; i<8; i++) {
		MDD += G::distance(CTR, EP[i]);
		CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << EP[i].x() << "\t" << EP[i].y() << "\t" << 256000 << "\t" << 0;
	}
	MDD = MDD/16;

	int k_max = 200;
	int k_min = 1;
	double Emin = 0;
	while (k_max - k_min > 1) {
		int k = (k_max + k_min) /2;
		for (int i=0; i<1; i++) trace1() << "k " << k << " k_max " << k_max << " k_min " << k_min;
		unordered_set<int> S;
		for (int i=0; i<numNodes; i++) {
			if (i == self) continue;
			S.insert(i);
		}
		vector<int> RP;
		vector<int> C;
		vector<int> nodes;
		nodes.insert(nodes.end(), S.begin(), S.end());
		for (int i=0; i<10; i++) trace1() << "kMeansClustering";
		auto Clusters = kMeansClustering(nodes, k);
		for (auto p : Clusters) C.push_back(p.first);
		double tourcost = 0;
		while (tourcost < L_max) {
			trace() << "RP " << RP.size() << " S " << S.size() << " C " << C.size() << " tourcost " << tourcost;
			for (int i=0; i<C.size(); i++) {
				if (Clusters[C[i]].size() <= 1) C.erase(std::remove(C.begin(), C.end(), C[i]), C.end());
			}
			vector<double> OHN (C.size());
			vector<double> AHD (C.size());
			vector<double> mod (C.size());
			double max_OHN = 0;
			double max_AHD = 0;
			double max_mod = 0;
			for (int i=0; i<C.size(); i++) {
				// OHN[i] = Clusters[C[i]].size();
				// for (int j : Clusters[C[i]]) {
				// 	AHD[i] += G::distance(location(C[i]), location(j));
				// }
				OHN[i] = 0;
				AHD[i] = 0;
				for (int j=0; j<numNodes; j++) {
					if (j == sinkId || j == i) continue;
					if (distance(C[i],j) <= d0) {
						OHN[i]++;
						AHD[i] += distance(C[i], j);
					}
				}
				AHD[i] /= OHN[i];
				double dist = G::distance(CTR, location(C[i]));
				mod[i] = abs(MDD - dist);//int(MDD/dist);

				if (OHN[i] > max_OHN) max_OHN = OHN[i];
				if (AHD[i] > max_AHD) max_AHD = AHD[i];
				if (mod[i] > max_mod) max_mod = mod[i];
			}

			vector<double> W (C.size());
			double Wmax = 0;
			int c_min;
			for (int i=0; i<C.size(); i++) {
				W[i] = OHN[i]/max_OHN / ((mod[i]/max_mod) * (AHD[i]/max_AHD));
				// trace1() << "C" << i << " " << W[i] << " OHN[i] " << OHN[i] << " max_OHN " << max_OHN << " mod[i] " << mod[i] << " max_mod " << max_mod << " AHD[i] " << AHD[i] << " max_AHD " << max_AHD;
				if (W[i] > Wmax) {
					Wmax = W[i];
					c_min = C[i];
				}
			}
			RP.push_back(c_min);
			C.erase(std::remove(C.begin(), C.end(), c_min), C.end());
			cent[c_min] = -1;
			// for (int i : Clusters[c_min]) {
			for (int i : S) {
				if (distance(i, c_min) > d0) continue;
				S.erase(i);
				cent[i] = c_min;
			}
			if (RP.size() > 1) {
				vector<int> RPfull;
				RPfull.push_back(self);
				RPfull.insert(RPfull.end(), RP.begin(), RP.end());
				trajectories[0] = mainTSP(RPfull);
				tourcost = calculatePathLength(trajectories[0]);
			}
			if (C.empty()) break;
		}

		for (int i=0; i<numNodes; i++) {
			if (cent[i] != -1) {
				debugLine(location(i), location(cent[i]), "black");
			} else {
				debugPoint(location(i), "red");
			}
		}
		vector<Point> T;
		for (int l : trajectories[0]) T.push_back(location(l));
		debugPath(T, "green");
		A = RP;
		isLandmark = vector<bool>(N, false);
		for (int l : A) isLandmark[l] = true;
		clearData();
		for (int i=0; i<10; i++) trace1() << "growBalls";
		growBalls(A);
		for (int l : A) cent[l] = -1;

		for (int i=0; i<10; i++) trace1() << "calculateConsumption";
		double E0 = calculateConsumption(trajectories);
		for (int i=0; i<10; i++) trace1() << "Emin " << Emin << " E0 " << E0;
		if (Emin < E0) {
			Emin = E0;
			k_min = k;
			config.save(A, cent, next, trajectories);
		}
		else k_max = k;
		
	}

	// vector<int> nodes;
	// for (int i=0; i<numNodes; i++) nodes.push_back(i);
	
	// int k;
	// int k_min = 2;
	// int k_max = numNodes/2;
	// bool fl;
	// int count = 0;
	// do {
	// 	trace() << "k " << k << " k_min " << k_min << " k_max " << k_max;
	// 	k = (k_max+k_min)/2;
	// 	if (k_opt != -1) {
	// 		k = k_max = k_min = k_opt;
	// 	}
	// 	A.clear();
	// 	auto Clusters = kMeansClustering(nodes, k);
	// 	for (auto p : Clusters) A.push_back(p.first);

	// 	vector<int> RPfull;
	// 	RPfull.push_back(self);
	// 	RPfull.insert(RPfull.end(), A.begin(), A.end());
	// 	vector<int> trajectory = mainTSP(RPfull);

	// 	fl = false;	
	// 	if (calculatePathLength(trajectory) > L_max) {
	// 		trace() << "pathLength " << calculatePathLength(trajectory);
	// 		fl = true;
	// 	};

	// 	if (fl) {
	// 		for (int i=0; i<1; i++) trace() << "reduce k_max";
	// 		k_max = k;
	// 	} else {
	// 		for (int i=0; i<1; i++) trace() << "save config";
	// 		clearData();
	// 		for (int l : A) isLandmark[l] = true;
	// 		growBalls(A);
	// 		trace() << "A.size " << A.size();
	// 		for (int l : A) cent[l] = -1;
	// 		trajectories[0] = trajectory;
	// 		config.save(A, cent, next, trajectories);
	// 		k_min = k;
	// 	}

	// 	if (k_max - k_min > 1){
			
	// 	} else if (fl) {
	// 		k_opt = -1;
	// 		k_min = 2;
	// 		k_max = numNodes/2;
	// 	}

	// } while (k_max - k_min > 1);
	// // if (k_opt == -1) k_opt = k;

	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;
	}
	trace1() << "maxLength " << maxLength;
	trace1() << "A size " << config.A.size();
}


void kaswan2016::init() {
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

void kaswan2016::reset() {
	
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

void kaswan2016::growBalls(vector<int> landmarkSet){
	// for (int i=0; i<20; i++) trace() << "growBalls Asize " << A.size();
	for (int u : landmarkSet) {
		dLandmark[u] = 0;
		representSet[u].clear();
		cent[u] = u;
		next[u] = -1;
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

void kaswan2016::clearData(){
	for (int u : graph.getNodesExceptSink()){
		isLandmark[u] = false;
		dLandmark[u] = DBL_MAX;
		cent[u] = -1;
		next[u] = -1;
		centList[u].clear();
		representSet[u].clear();
	}
}

void kaswan2016::alphaHopClustering(int alpha){
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

void kaswan2016::buildTrajectories(){
	for (int i=0; i<numUAVs; i++) trajectories[i].clear();
	if (A.size() < numUAVs) {
		for (int i=0; i<A.size(); i++) {
			trajectories[i].push_back(self);
			trajectories[i].push_back(A[i]);
			trajectories[i].push_back(self);
		}
	}
	else {
		unordered_set<int> Unvisited;
		Unvisited.insert(A.begin(), A.end());
		int current = self;
		int i = 0;
		while (!Unvisited.empty() && (i < numUAVs)) {
			double L = 0;
			trajectories[i].push_back(current);
			bool isFinished = true;
			while (isFinished) {
				double dmin = DBL_MAX;
				int candidate = -1;
				for (int next : Unvisited) {
					double d = G::distance(location(current), location(next));
					if (d < dmin) {
						dmin = d;
						candidate = next;
					}
				}

				if (L+dmin+G::distance(location(candidate), selfLocation) <= L_max) {
					L += dmin;
					trajectories[i].push_back(candidate);
					current = candidate;
					Unvisited.erase(current);
				} else {
					i++;
					current = self;
					isFinished = false;
				}
			}
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

double kaswan2016::calculateConsumption(vector<vector<int>> tours) {
	// for (int i=0; i<10; i++) trace1() << "calculateConsumption";
	NetworkConfig oldConfig;
	oldConfig.save(config);
	config.save(A, cent, next, trajectories);

	double Er_min = DBL_MAX;
	double Er_total = 0;
	double new_cnsmptn = 0;
	for (int i=0; i<numNodes; i++) {
		double Er_i = getResMgrModule(i)->getRemainingEnergy();
		if (i == sinkId) continue;
		double rxSize = calculateRxSize(i);
		// trace1() << "Er_" << i << " " << Er_i << " " << rxSize << " " << weights[i];
		new_cnsmptn += rxEnergy(rxSize);
		Er_i -= rxEnergy(rxSize);
		// trace1() << "rx " << rxEnergy(rxSize);
		double txSize = rxSize + weights[i];
		if (cent[i] == -1) {
			new_cnsmptn += txEnergy(txSize, D2UAV);
			Er_i -= txEnergy(txSize, D2UAV);
			// trace1() << "tx " << txEnergy(txSize, D2UAV);
		} else {
			double d2next = distance(i, next[i]);
			new_cnsmptn += txEnergy(txSize, d2next);
			Er_i -= txEnergy(txSize, d2next);
			// trace1() << "tx " << txEnergy(txSize, d2next);
		}
		Er_total += Er_i;
		if (Er_i < Er_min) Er_min = Er_i;
	}
	config.save(oldConfig);
	// for (int i=0; i<10; i++) trace1() << "calculateConsumption end";
	return Er_min;
	// return new_cnsmptn;
}